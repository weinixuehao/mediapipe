#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "tensorflow/lite/c/common.h"
#if !defined(MEDIAPIPE_DISABLE_GL_COMPUTE)
#include "tensorflow/lite/delegates/gpu/gl/gl_buffer.h"
#endif

#if defined(MEDIAPIPE_IOS)
// #import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>
// #import <MetalKit/MetalKit.h>

// #import "mediapipe/gpu/MPPMetalHelper.h"
// #include "mediapipe/gpu/MPPMetalUtil.h"
// #include "mediapipe/gpu/gpu_buffer.h"
// #include "tensorflow/lite/delegates/gpu/metal_delegate.h"
#endif // iOS

#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/gpu/shader_util.h"
#include <typeinfo>
#include <ctime>
#include <string>
#include "fm_ocr_scanner.h"
#include <cstdlib>
#define DETECT_THRESHOLD 14

namespace mediapipe
{

    namespace
    {
        constexpr uchar kAnnotationBackgroundColor = 2; // Grayscale value.
        enum
        {
            ATTRIB_VERTEX,
            ATTRIB_TEXTURE_POSITION,
            NUM_ATTRIBUTES
        };
        size_t RoundUp(size_t n, size_t m) { return ((n + m - 1) / m) * m; } // NOLINT
    }                                                                        // namespace

    class HedEdgeDetectionCalculator : public CalculatorBase
    {
    public:
        static ::mediapipe::Status GetContract(CalculatorContract *cc);

        ::mediapipe::Status Open(CalculatorContext *cc) override;
        ::mediapipe::Status Process(CalculatorContext *cc) override;
        ::mediapipe::Status Close(CalculatorContext *cc) override;
        ::mediapipe::Status GlSetup(CalculatorContext *cc);
        ::mediapipe::Status GlRender(CalculatorContext *cc);
        ::mediapipe::Status renderToGpu(CalculatorContext *cc, uchar *overlay_image);
        ::mediapipe::Status tensorToOverlay(CalculatorContext *cc, const std::vector<TfLiteTensor> &input_tensors, std::unique_ptr<cv::Mat> &image_mat);
        bool reliableRect(std::vector<cv::Point> &point);

    private:
        mediapipe::GlCalculatorHelper gpu_helper_;
        GLuint image_mat_tex_ = 0; //show camera frame.
        bool gpu_initialized_ = false;
        GLuint program_ = 0;
        int width_ = 0;
        int height_ = 0;
        float ratio_x = 0;
        float ratio_y = 0;
        int invalid_rect_count = 0;
        int undetect_count = 0;
    };

    ::mediapipe::Status HedEdgeDetectionCalculator::GetContract(CalculatorContract *cc)
    {
        cc->Inputs().Index(0).Set<std::vector<TfLiteTensor>>();
        cc->Inputs().Tag("IMAGE_GPU").Set<mediapipe::GpuBuffer>();
        cc->Outputs().Tag("IMAGE_GPU").Set<mediapipe::GpuBuffer>();
        cc->Outputs().Tag("CONTOUR_STATUS").Set<int>();
        MP_RETURN_IF_ERROR(mediapipe::GlCalculatorHelper::UpdateContract(cc));
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::Open(CalculatorContext *cc)
    {
        MP_RETURN_IF_ERROR(gpu_helper_.Open(cc));
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::GlSetup(CalculatorContext *cc)
    {
        const GLint attr_location[NUM_ATTRIBUTES] = {
            ATTRIB_VERTEX,
            ATTRIB_TEXTURE_POSITION,
        };
        const GLchar *attr_name[NUM_ATTRIBUTES] = {
            "position",
            "texture_coordinate",
        };

        const GLchar *vert_src = R"(
            #if __VERSION__ < 130
            #define in attribute
            #define out varying
            #endif
            in vec4 position;
            in mediump vec4 texture_coordinate;
            out mediump vec2 sample_coordinate;
            void main() {
                gl_Position = position;
                sample_coordinate = texture_coordinate.xy;
            }
        )";

        const GLchar *frag_src = R"(
            #if __VERSION__ < 130
                #define in varying
            #endif  // __VERSION__ < 130

            #ifdef GL_ES
                #define fragColor gl_FragColor
                precision highp float;
            #else
                #define lowp
                #define mediump
                #define highp
                #define texture2D texture
                out vec4 fragColor;
            #endif  // defined(GL_ES)
            in vec2 sample_coordinate;
            uniform sampler2D input_frame;
            uniform sampler2D overlay;
            uniform vec3 transparent_color;
            void main() {
                vec4 image_pix = texture2D(input_frame, sample_coordinate);
                vec4 overlay_pix = texture2D(overlay, sample_coordinate);
                float dist = distance(overlay_pix.rgb, transparent_color);
                if (dist > 0.001) {
                    fragColor = mix(overlay_pix, image_pix, 0.7);
                } else {
                    fragColor = image_pix;
                }
            }
        )";

        mediapipe::GlhCreateProgram(vert_src, frag_src,
                                    NUM_ATTRIBUTES, (const GLchar **)&attr_name[0],
                                    attr_location, &program_);
        RET_CHECK(program_) << "Problem initializing the program.";
        glUseProgram(program_);
        glUniform1i(glGetUniformLocation(program_, "input_frame"), 1);
        glUniform1i(glGetUniformLocation(program_, "overlay"), 2);
        glUniform3f(glGetUniformLocation(program_, "transparent_color"),
                    kAnnotationBackgroundColor / 255.0,
                    kAnnotationBackgroundColor / 255.0,
                    kAnnotationBackgroundColor / 255.0);
        const auto &input_frame =
            cc->Inputs().Tag("IMAGE_GPU").Get<mediapipe::GpuBuffer>();
        width_ = RoundUp(input_frame.width(), ImageFrame::kGlDefaultAlignmentBoundary);
        height_ = RoundUp(input_frame.height(), ImageFrame::kGlDefaultAlignmentBoundary);
        ratio_y = height_ / 256.0;
        ratio_x = width_ / 256.0;
        {
            glGenTextures(1, &image_mat_tex_);
            glBindTexture(GL_TEXTURE_2D, image_mat_tex_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width_, height_, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::GlRender(CalculatorContext *cc)
    {
#if !defined(MEDIAPIPE_DISABLE_GPU)
        static const GLfloat square_vertices[] = {
            -1.0f, -1.0f, // bottom left
            1.0f, -1.0f,  // bottom right
            -1.0f, 1.0f,  // top left
            1.0f, 1.0f,   // top right
        };
        static const GLfloat texture_vertices[] = {
            0.0f, 0.0f, // bottom left
            1.0f, 0.0f, // bottom right
            0.0f, 1.0f, // top left
            1.0f, 1.0f, // top right
        };

        // program
        glUseProgram(program_);

        // vertex storage
        GLuint vbo[2];
        glGenBuffers(2, vbo);
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // vbo 0
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), square_vertices,
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(ATTRIB_VERTEX);
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, nullptr);

        // vbo 1
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), texture_vertices,
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(ATTRIB_TEXTURE_POSITION);
        glVertexAttribPointer(ATTRIB_TEXTURE_POSITION, 2, GL_FLOAT, 0, 0, nullptr);

        // draw
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // cleanup
        glDisableVertexAttribArray(ATTRIB_VERTEX);
        glDisableVertexAttribArray(ATTRIB_TEXTURE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(2, vbo);
#endif //  !MEDIAPIPE_DISABLE_GPU
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::tensorToOverlay(CalculatorContext *cc, const std::vector<TfLiteTensor> &input_tensors, std::unique_ptr<cv::Mat> &image_mat)
    {
        auto &hed_tensor = input_tensors[0];
        float *hed_buf = hed_tensor.data.f;
        cv::Mat hed_img(256, 256, CV_32FC1, hed_buf);
        auto tuple = ProcessEdgeImage(hed_img, hed_img, true, false);
        auto find_rect = std::get<0>(tuple);
        auto cv_points = std::get<1>(tuple);
        static int count = 0;
        if (find_rect)
        {
            this->undetect_count = 0;
            cv::Mat *img = image_mat.get();
            std::vector<cv::Point> real_points(4);
            for (int i = 0, len = cv_points.size(); i < len; i++)
            {
                cv::Point real_point(cv_points[i].x * ratio_x, cv_points[i].y * ratio_y);
                real_points[i] = real_point;
            }
            bool reliableRect = this->reliableRect(real_points);
            if (reliableRect) {
                ++count;
                this->invalid_rect_count = 0;
            } else {
                this->invalid_rect_count++;
            }
            cv::fillConvexPoly(*img, real_points, reliableRect ? cv::Scalar(0, 255, 0) : cv::Scalar(255, 0, 0));
            if (count > DETECT_THRESHOLD) {
                std::unique_ptr<int> result = absl::make_unique<int>(0);
                cc->Outputs().Tag("CONTOUR_STATUS").Add(result.release(), cc->InputTimestamp());
                count = 0;
            } else if (count > 3 && reliableRect) {
                std::unique_ptr<int> result = absl::make_unique<int>(1);
                cc->Outputs().Tag("CONTOUR_STATUS").Add(result.release(), cc->InputTimestamp());
            } else if (this->invalid_rect_count > 4){
                std::unique_ptr<int> result = absl::make_unique<int>(2);
                cc->Outputs().Tag("CONTOUR_STATUS").Add(result.release(), cc->InputTimestamp());
            }
        } else {
            this->undetect_count++;
            if (this->undetect_count > 3) {
                count = 0;
            }
        }
        return ::mediapipe::OkStatus();
    }

    bool HedEdgeDetectionCalculator::reliableRect(std::vector<cv::Point> &points) {
        int topSide = points[1].x - points[0].x;
        int bottomSide = points[2].x - points[3].x;
        int leftSide = points[3].y - points[0].y;
        int rightSide = points[2].y - points[1].y;
        size_t perimeter = (width_ + height_) * 2;
        float times = perimeter / (float)(topSide + rightSide + bottomSide + leftSide);
        if (times > 3) {
            return false;
        }
        return std::abs(topSide-bottomSide) < 130 && std::abs(leftSide - rightSide) < 130;
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::renderToGpu(CalculatorContext *cc, uchar *overlay_image)
    {
        const auto &input_frame =
            cc->Inputs().Tag("IMAGE_GPU").Get<mediapipe::GpuBuffer>();
        auto input_texture = gpu_helper_.CreateSourceTexture(input_frame);
        auto output_texture = gpu_helper_.CreateDestinationTexture(width_, height_, mediapipe::GpuBufferFormat::kBGRA32);

        // Upload render target to GPU.
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindTexture(GL_TEXTURE_2D, image_mat_tex_);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGB,
                            GL_UNSIGNED_BYTE, overlay_image);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
        }

        {
            gpu_helper_.BindFramebuffer(output_texture); // GL_TEXTURE0

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, input_texture.name());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, image_mat_tex_);

            MP_RETURN_IF_ERROR(GlRender(cc));
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFlush();
        }

        // Send out blended image as GPU packet.
        auto output_frame = output_texture.GetFrame<mediapipe::GpuBuffer>();
        cc->Outputs()
            .Tag("IMAGE_GPU")
            .Add(output_frame.release(), cc->InputTimestamp());

        // Cleanup
        input_texture.Release();
        output_texture.Release();
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::Process(CalculatorContext *cc)
    {
        if (!gpu_initialized_)
        {
            MP_RETURN_IF_ERROR(
                gpu_helper_.RunInGlContext([this, cc]() -> ::mediapipe::Status {
                    MP_RETURN_IF_ERROR(GlSetup(cc));
                    return ::mediapipe::OkStatus();
                }));
            gpu_initialized_ = true;
        }
        const auto &input_tensors = cc->Inputs().Index(0).Get<std::vector<TfLiteTensor>>();
        MP_RETURN_IF_ERROR(gpu_helper_.RunInGlContext([this, &input_tensors, cc]() -> ::mediapipe::Status {
            std::unique_ptr<cv::Mat> image_mat;
            image_mat = absl::make_unique<cv::Mat>(height_, width_, CV_8UC3);
            memset(image_mat->data, kAnnotationBackgroundColor,
                   height_ * width_ * image_mat->elemSize());
            tensorToOverlay(cc, input_tensors, image_mat);
            renderToGpu(cc, image_mat->data);
            return ::mediapipe::OkStatus();
        }));

        return ::mediapipe::OkStatus();
    }
    ::mediapipe::Status HedEdgeDetectionCalculator::Close(CalculatorContext *cc)
    {
        gpu_helper_.RunInGlContext([this] {
            if (program_)
                glDeleteProgram(program_);
            program_ = 0;
            if (image_mat_tex_)
                glDeleteTextures(1, &image_mat_tex_);
            image_mat_tex_ = 0;
        });

        return ::mediapipe::OkStatus();
    }
    REGISTER_CALCULATOR(HedEdgeDetectionCalculator);
} // namespace mediapipe
