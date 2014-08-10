#ifndef VIDEO_PROCESSOR_HPP
#define VIDEO_PROCESSOR_HPP

#include <CImg.h>
#include "image_process.hpp"

using namespace cimg_library;

#include <string>

class video_worker {
    public:
        virtual void process_frame(const CImg8 &frame, int frame_count) = 0;
};

class video_processor {
    public:
        int iterate(
            video_worker &worker,
            std::string video_path,
            int start_from=0, 
            int start_from_back=0,
            int frame_skip=0,
            int width=0,
            int height=0);
};

#endif
