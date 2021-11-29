#ifndef INFERENCE_H
#define INFERENCE_H
#include <string>
#include "tensorflow/lite/interpreter.h"
#include "mediapipe/framework/deps/status.h"
#include <opencv2/core/mat.hpp>

class HedEdgeDetecion
{
public:
    using TfLiteDelegatePtr =
        std::unique_ptr<TfLiteDelegate, std::function<void(TfLiteDelegate*)>>;
    HedEdgeDetecion(std::string &model_path);
    ~HedEdgeDetecion();
    ::mediapipe::Status run(cv::Mat &inputImg, std::vector<cv::Point> &points);
private:
    std::unique_ptr<tflite::Interpreter> interpreter_;
    TfLiteDelegatePtr delegate_;
    ::mediapipe::Status init(std::string &model_path);
    void readImg(cv::Mat &origImg);
};

#endif // INFERENCE_H
