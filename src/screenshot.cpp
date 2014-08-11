#define __STDC_CONSTANT_MACROS

#include <CImg.h>

using namespace cimg_library;

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

#include <boost/program_options.hpp>

#include "image_process.hpp"
#include "video_process.hpp"

struct screenshot_context {
    int start_from;
    int frame_skip;
    int screenshot_limit;
    int time_limit;

    std::string video_path;
    std::string screenshot_path;

    int parse_command_line(int argc, char** argv) {
        namespace po = boost::program_options;
        po::options_description desc("Options");

        desc.add_options()
            ("start-from",       po::value<int>()->default_value(60),   "start video this many seconds from the end")
            ("frame-skip",       po::value<int>()->default_value(5),    "skip frames during video processing")
            ("screenshot-limit", po::value<int>()->default_value(500),  "histogram limit to keep a screenshot")
            ("time-limit",       po::value<int>()->default_value(60*2), "time in seconds to search the video")
            ("video",            po::value<std::string>(),              "video file to screenshot")
            ("screenshot",       po::value<std::string>(),              "output image"); 

        po::positional_options_description pos_desc;
        pos_desc.add("video", 1);
        pos_desc.add("screenshot", 1);
        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(),  vm);
            po::notify(vm);
        } catch(po::error& e) {
            std::cerr << "Error during parsing" << std::endl;
            std::cerr << desc;
            return 1;
        }

        this->start_from      = vm["start-from"].as<int>();
        this->screenshot_limit = vm["screenshot-limit"].as<int>();
        this->time_limit      = vm["time-limit"].as<int>();
        this->frame_skip      = vm["frame-skip"].as<int>();
        this->video_path      = vm["video"].as<std::string>();
        this->screenshot_path = vm["screenshot"].as<std::string>();

        return 0;
    }
};

class screenshot_video_worker : public video_worker {
    const  screenshot_context &sc;
    CImg8  current_screenshot;
    bool   found_good_screenshot;

    public:
        virtual bool process_frame(const CImg8 &frame, int frame_count, int curr_time);

        const CImg8 &get_screenshot() {
            return current_screenshot;
        }

        const bool get_found_good_screenshot() {
            return found_good_screenshot;
        }

        screenshot_video_worker(const screenshot_context &sc) : sc(sc), found_good_screenshot(false) {
        
        }
};

bool screenshot_video_worker::process_frame(const CImg8 &frame, int frame_count, int curr_time) {
    if(get_unique_colors(frame) >= sc.screenshot_limit) {
        found_good_screenshot = true;
        current_screenshot = frame;
    }

    return true;
}

int main(int argc, char *argv[]) {
    screenshot_context sc;

    sc.parse_command_line(argc, argv);

    screenshot_video_worker worker(sc);
    video_processor processor;

    processor.iterate(
        worker,
        sc.video_path,
        0,
        sc.start_from,
        sc.frame_skip,
        0,
        0);

    if (worker.get_found_good_screenshot()) {
        worker.get_screenshot().save(sc.screenshot_path.c_str());
    } else {
        std::cerr << "could not find good screenshot" << endl;
    }
}
