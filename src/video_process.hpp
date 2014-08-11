#ifndef VIDEO_PROCESSOR_HPP
#define VIDEO_PROCESSOR_HPP

#define __STDC_CONSTANT_MACROS

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include <CImg.h>
#include "image_process.hpp"

using namespace cimg_library;

#include <string>

class video_worker {
    public:
        virtual bool process_frame(const CImg8 &frame, int frame_count, int curr_time) = 0; // return true to continue processing, false to stop
};

class video_processor {
    protected:
        void print_error(int err, const std::string &message);
    public:
        int iterate(
            video_worker &worker,
            std::string video_path,
            int start_from=0, 
            int start_from_back=0,
            int frame_skip=0,
            int width=0,
            int height=0
        );
};

#endif
