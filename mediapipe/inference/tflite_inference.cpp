#include "tflite_inference.h"
#include "tensorflow/lite/kernels/register.h"
#include "mediapipe/util/resource_util.h"
#include "tensorflow/lite/delegates/gpu/gl_delegate.h"
#include "mediapipe/framework/deps/status_macros.h"
#include "mediapipe/framework/deps/ret_check.h"

TfliteInference::TfliteInference(std::string &model_path){
    mediapipe::Status status = this->init(model_path);
}

::mediapipe::Status TfliteInference::init(std::string &model_path) {
    auto actual_path = std::move(mediapipe::PathToResourceAsFile(model_path)).ValueOrDie();
    auto model = tflite::FlatBufferModel::BuildFromFile(actual_path.c_str());
    RET_CHECK(model) << "Failed to load model from path.";
    tflite::ops::builtin::BuiltinOpResolver op_resolver;
    tflite::InterpreterBuilder(*model.get(), op_resolver)(&interpreter_);
    interpreter_->AllocateTensors();

    TfLiteGpuDelegateOptions options = TfLiteGpuDelegateOptionsDefault();
    options.compile_options.precision_loss_allowed = 1;
    options.compile_options.preferred_gl_object_type =
        TFLITE_GL_OBJECT_TYPE_FASTEST;
    options.compile_options.dynamic_batch_enabled = 0;
    options.compile_options.inline_parameters = 1;

    if (!delegate_)
      delegate_ = TfLiteDelegatePtr(TfLiteGpuDelegateCreate(&options),
                                    &TfLiteGpuDelegateDelete);
    interpreter_->ModifyGraphWithDelegate(delegate_.get());
    return ::mediapipe::OkStatus();
}

void TfliteInference::run() {
    interpreter_->Invoke();
}

TfliteInference::~TfliteInference() {

}

