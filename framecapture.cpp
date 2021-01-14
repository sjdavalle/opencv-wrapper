#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "framecapture.h"

// Opaque structure to hide the cv::VideoCapture attribute and do not depend on OpenCV during compilation time
struct CaptureDevice
{
    cv::VideoCapture dev;
};

FrameCapture::FrameCapture(const int device_id, const int w, const int h): 
    _device_id(device_id), _w(w), _h(h)
{
    cap = new CaptureDevice();
    if(!cap->dev.open(device_id))
        throw std::runtime_error("Error opening device " + std::to_string(device_id));
    
    _resize = (w != 0 && h != 0);
}

FrameCapture::~FrameCapture()
{
    if(cap)
    {
        cap->dev.release();
    }
}

bool FrameCapture::isOpened() const
{
    return cap->dev.isOpened();
}

ImageData FrameCapture::getFrame()
{
    ImageData data{0};
    cv::Mat orig_frame;
    cv::Mat resized_mat;

    const bool ret = cap->dev.read(orig_frame);

    if(ret)
    {
        if(_resize)
        {
            cv::resize(orig_frame, resized_mat, cv::Size(_w,_h));
        }
        cv::imencode(".jpg", _resize ? resized_mat : orig_frame, data.vec);
        data.image_size = data.vec.size();
        data.device_id = _device_id;

        orig_frame.release();
        resized_mat.release();
    }
    return data;
}

void FrameHandler::showImage(ImageData data)
{
    cv::Mat restored = cv::imdecode(data.vec, 1);
    std::string display_name {"Capture from device: " + std::to_string(data.device_id)};
    cv::imshow(display_name.c_str(), restored);
    cv::waitKey(1);
}

FrameHandler::~FrameHandler()
{
    cv::destroyAllWindows();
}