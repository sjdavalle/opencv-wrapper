#pragma once

struct ImageData
{
    int device_id;
    size_t image_size;
    std::vector<unsigned char> vec;
};

struct CaptureDevice;

class FrameCapture
{
    public:
        FrameCapture(const int device_id, const int w = 0, const int h = 0);
        ~FrameCapture();
        bool isOpened() const;
        ImageData getFrame();

    private:
        CaptureDevice *cap;
        int _device_id;
        int _w;
        int _h;
        bool _resize;
};

class FrameHandler
{
    public:
        FrameHandler(){}
        ~FrameHandler();
        void showImage(ImageData data);
};