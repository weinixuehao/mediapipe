#ifndef INFERENCE_H
#define INFERENCE_H
#include <string>
#include "tensorflow/lite/interpreter.h"
#include "mediapipe/framework/deps/status.h"

class TfliteInference
{
public:
    using TfLiteDelegatePtr =
        std::unique_ptr<TfLiteDelegate, std::function<void(TfLiteDelegate*)>>;
    TfliteInference(std::string &model_path);
    ~TfliteInference();
    void run();
private:
    std::unique_ptr<tflite::Interpreter> interpreter_;
    TfLiteDelegatePtr delegate_;
    ::mediapipe::Status init(std::string &model_path);
};

#endif // INFERENCE_H
