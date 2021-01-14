#include <iostream>
#include <thread>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string>
#include <mutex>
#include <queue>
#include <atomic>

#include "framecapture.h"

volatile sig_atomic_t do_exit = 0;

static void set_do_exit(int sig) 
{
  do_exit = 1;
}

constexpr int max_length = 32;

std::queue<ImageData> buffer;
std::mutex mtxCam;

static void bufferStore(ImageData frame)
{
    mtxCam.lock();
    buffer.push(frame);
    mtxCam.unlock();
}

void sendFrame()
{
    std::size_t bufSize {0};  
    std::cout << "Send Frames thread started!! " << std::endl;
    FrameHandler handler;
    while (!do_exit)
    {
        ImageData frame_data;
        mtxCam.lock();            
        bufSize = buffer.size(); 
        if (bufSize > 0)         
        {
            frame_data = buffer.front();
            buffer.pop();
            std::cout << "packet obtained - bufsize = " << bufSize << std::endl;
        }
        mtxCam.unlock();
        if (!frame_data.vec.empty())
        {
            
            int num_of_packs = 1;
            if (frame_data.image_size > max_length)
            {
                num_of_packs = ceil(frame_data.image_size/max_length);
            }

            std::cout << "Packs to send: " << num_of_packs << std::endl;
            //AUTOSAR send num_of_packs
            //AUTOSAR send frame_data.vec
            handler.showImage(frame_data);
        }
    }

    std::cout << "exiting sendFrame thread" << std::endl;
}

void getFrame(int cam_id) 
{
    ImageData tmp;
    std::cout << "Get Frames thread started!! CAMID: " << cam_id << std::endl;

    try
    {
        FrameCapture capture(cam_id);
        if(capture.isOpened())
        {
            while (!do_exit)
            {
                tmp = capture.getFrame();
                if (tmp.vec.empty()) 
                {
                    std::cout << "empty frame" << std::endl;
                    continue;
                }
                bufferStore(tmp);
                std::cout << "frame stored" << std::endl;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "EXCEPTION!: " << e.what() << '\n';
    }

    std::cout << "exiting getFrame thread" << std::endl;
}

int main(int argc, char* argv[]) 
{
    signal(SIGINT, (sighandler_t)set_do_exit);
    signal(SIGTERM, (sighandler_t)set_do_exit);

    std::thread framethrd_0(getFrame, 0);
    std::thread framethrd_1(getFrame, 1);
    std::thread sendFrameThrd(sendFrame);

    framethrd_0.join();
    framethrd_1.join();
    sendFrameThrd.join();

    std::cout << "exiting main thread" << std::endl;
}