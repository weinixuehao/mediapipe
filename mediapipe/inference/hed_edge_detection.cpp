#include "hed_edge_detection.h"
#include "tensorflow/lite/kernels/register.h"
#include "mediapipe/util/resource_util.h"
#ifdef __ANDROID__
#include "tensorflow/lite/delegates/gpu/gl_delegate.h"
#else
//#import <CoreVideo/CoreVideo.h>
//#import <Metal/Metal.h>
//#import <MetalKit/MetalKit.h>
//#include "tensorflow/lite/delegates/gpu/common/shape.h"
//#include "tensorflow/lite/delegates/gpu/metal/buffer_convert.h"
#include "tensorflow/lite/delegates/gpu/metal_delegate.h"
//#include "tensorflow/lite/delegates/gpu/metal_delegate_internal.h"
#endif
#include "mediapipe/framework/deps/status_macros.h"
#include "mediapipe/framework/deps/ret_check.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "mediapipe/calculators/custom/fm_ocr_scanner.h"

#define WIDTH 256
#define HEIGHT 256

HedEdgeDetecion::HedEdgeDetecion(std::string &model_path){
    mediapipe::Status status = this->init(model_path);
}

::mediapipe::Status HedEdgeDetecion::init(std::string &model_path) {
    auto actual_path = std::move(mediapipe::PathToResourceAsFile(model_path)).ValueOrDie();
    auto model = tflite::FlatBufferModel::BuildFromFile(actual_path.c_str());
    RET_CHECK(model) << "Failed to load model from path.";
    tflite::ops::builtin::BuiltinOpResolver op_resolver;
    tflite::InterpreterBuilder(*model.get(), op_resolver)(&interpreter_);
    interpreter_->AllocateTensors();

    if (!delegate_) {
#ifdef __ANDROID__
        TfLiteGpuDelegateOptions options = TfLiteGpuDelegateOptionsDefault();
        options.compile_options.precision_loss_allowed = 1;
        options.compile_options.preferred_gl_object_type =
                TFLITE_GL_OBJECT_TYPE_FASTEST;
        options.compile_options.dynamic_batch_enabled = 0;
        options.compile_options.inline_parameters = 1;
        delegate_ = TfLiteDelegatePtr(TfLiteGpuDelegateCreate(&options),
                                      &TfLiteGpuDelegateDelete);
#else
        TFLGpuDelegateOptions options;
        options.allow_precision_loss = true;
        options.wait_type = TFLGpuDelegateWaitType::TFLGpuDelegateWaitTypePassive;
        delegate_ = TfLiteDelegatePtr(TFLGpuDelegateCreate(&options),
                                      &TFLGpuDelegateDelete);
#endif
    }
    interpreter_->ModifyGraphWithDelegate(delegate_.get());
    return ::mediapipe::OkStatus();
}

void HedEdgeDetecion::readImg(cv::Mat &origImg) {
    cv::Size size(WIDTH, HEIGHT);
    cv::Mat resizeImg;
    cv::resize(origImg, resizeImg, size, 0, 0, cv::INTER_LINEAR);
    cv::Mat rgbImage;
    cv::cvtColor(resizeImg, rgbImage, cv::COLOR_RGBA2RGB);
    assert(rgbImage.type() == CV_8UC3);
    /**
     void convertTo( OutputArray m, int rtype, double alpha=1, double beta=0 ) const;
     */
    rgbImage.convertTo(origImg, CV_32FC3, 1.0 / 255);
}

::mediapipe::Status HedEdgeDetecion::run(cv::Mat &inputImg, std::vector<cv::Point> &points) {
    this->readImg(inputImg);
    auto network_input = interpreter_->inputs()[0];
    int numOfFloat = sizeof(float);
    assert(numOfFloat == 4);
    float *network_input_ptr = interpreter_->typed_tensor<float>(network_input);
    const float *source_data = (float*) inputImg.data;
    std::memcpy(network_input_ptr, source_data, WIDTH*HEIGHT*3*sizeof(float));
    RET_CHECK_EQ(interpreter_->Invoke(), kTfLiteOk);
    float edge_map[WIDTH*HEIGHT*1];
    float *network_output = interpreter_->typed_output_tensor<float>(0);
    std::memcpy(edge_map, network_output, WIDTH*HEIGHT*1*sizeof(float));
    cv::Mat out_img = cv::Mat(HEIGHT, WIDTH, CV_32FC1, edge_map);
    auto tuple = ProcessEdgeImage(out_img, out_img, true, false);
    bool find_rect = std::get<0>(tuple);
    if (find_rect) {
        points = std::get<1>(tuple);
    }
    return ::mediapipe::OkStatus();
}

HedEdgeDetecion::~HedEdgeDetecion() {

}

