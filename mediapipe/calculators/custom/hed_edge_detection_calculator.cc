#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "tensorflow/lite/delegates/gpu/gl/gl_buffer.h"
#include "mediapipe/gpu/gl_calculator_helper.h"
#include <android/log.h>
#include <typeinfo>

typedef ::tflite::gpu::gl::GlBuffer GpuTensor;
namespace mediapipe
{
    class HedEdgeDetectionCalculator : public CalculatorBase
    {
        public:
            static ::mediapipe::Status GetContract(CalculatorContract *cc);

            ::mediapipe::Status Open(CalculatorContext *cc) override;
            ::mediapipe::Status Process(CalculatorContext *cc) override;
            ::mediapipe::Status Close(CalculatorContext *cc) override;
            mediapipe::GlCalculatorHelper gpu_helper_;
    };

    ::mediapipe::Status HedEdgeDetectionCalculator::GetContract(CalculatorContract *cc)
    {
        // __android_log_print(ANDROID_LOG_WARN, "hededge", "GetContract")
//        bool hasTag = cc->Inputs().HasTag("TENSORS_GPU");
//        cc->Inputs().Tag("TENSORS_GPU").Set<std::vector<GpuTensor>>();
        cc->Inputs().Index(0).Set<std::vector<GpuTensor>>();
        cc->Outputs().Tag("IMAGE_GPU").Set<mediapipe::GpuBuffer>();
        MP_RETURN_IF_ERROR(mediapipe::GlCalculatorHelper::UpdateContract(cc));
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::Open(CalculatorContext *cc)
    {
        MP_RETURN_IF_ERROR(gpu_helper_.Open(cc));
        return ::mediapipe::OkStatus();
    }

    ::mediapipe::Status HedEdgeDetectionCalculator::Process(CalculatorContext *cc)
    {
        const auto& input_tensors = cc->Inputs().Index(0).Get<std::vector<GpuTensor>>();
        MP_RETURN_IF_ERROR(gpu_helper_.RunInGlContext([this, &input_tensors, &cc]() -> ::mediapipe::Status {
            auto size = input_tensors.size();
            auto &hed_map_buf = input_tensors[0];
            const char *name = typeid(hed_map_buf).name();
            size_t float_num = hed_map_buf.bytes_size() / 4;
            std::vector<float> boxes(float_num);
            hed_map_buf.Read(absl::MakeSpan(boxes));
            for(std::vector<float>::iterator it = boxes.begin(); it != boxes.end(); ++it) {
                __android_log_print(ANDROID_LOG_WARN, "jni", "float_num=%zu, float=%f", float_num, *it);
            }
            return ::mediapipe::OkStatus();                                                        
        }));
        // const char *name = typeid(input_tensors).name();
        // __android_log_print(ANDROID_LOG_WARN, "jni", "hed_contour=%s", name);

        return ::mediapipe::OkStatus();
    }
    ::mediapipe::Status HedEdgeDetectionCalculator::Close(CalculatorContext *cc)
    {
        return ::mediapipe::OkStatus();
    }
    REGISTER_CALCULATOR(HedEdgeDetectionCalculator);
} // namespace mediapipe