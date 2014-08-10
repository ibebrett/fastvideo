#define __STDC_CONSTANT_MACROS

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include <CImg.h>

using namespace cimg_library;

#include <string>
#include <sstream>
#include <iostream>
#include <set>
#include <vector>
#include <cmath>
#include <list>

#include <boost/program_options.hpp>

#include "image_process.hpp"
#include "video_process.hpp"

struct search_context {
    int time_limit;
    int compare_limit;
    int screenshot_limit;
    int match_limit;
    int width;
    int height;
    bool debug_matches;

    std::vector<std::string> screenshot_paths;
    std::string video_path;

    int parse_command_line(int argc, char** argv) {
        namespace po = boost::program_options;
        po::options_description desc("Options");

        desc.add_options()
            ("time-limit",       po::value<int>()->default_value(5*60), "maximum time to search video for screenshots")
            ("compare-limit",    po::value<int>()->default_value(4),    "histogram value threshold for comparing screenshots to video")
            ("screenshot-limit", po::value<int>()->default_value(10),   "histogram value threshold to include screenshot in comparisons")
            ("match-limit",      po::value<int>()->default_value(500),  "value to consider a screenshot and frame matchign")
            ("width",            po::value<int>()->default_value(30),   "width when rescaling images")
            ("height",           po::value<int>()->default_value(30),   "height when rescaling images")
            ("debug-matches",    po::bool_switch(),                     "save matching screenshots")
            ("video",            po::value<std::string>(),  "video file")
            ("screenshots",      po::value<std::vector<std::string> >(), "screenshots"); 

        po::positional_options_description pos_desc;
        pos_desc.add("video", 1);
        pos_desc.add("screenshots", -1);
        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(),  vm);
            po::notify(vm);
        } catch(po::error& e) {
            std::cerr << "Error during parsing" << std::endl;
            std::cerr << desc;
            return 1;
        }

        this->time_limit       = vm["time-limit"].as<int>();
        this->compare_limit    = vm["compare-limit"].as<int>();
        this->screenshot_limit = vm["screenshot-limit"].as<int>();
        this->match_limit      = vm["match-limit"].as<int>();
        this->width            = vm["width"].as<int>();
        this->height           = vm["height"].as<int>();
        this->debug_matches    = vm["debug-matches"].as<bool>();
        this->video_path       = vm["video"].as<std::string>();
        this->screenshot_paths = vm["screenshots"].as<std::vector<std::string> >();

        return 0;
    }
};

class search_video_worker : public video_worker {
    const search_context &sc;
    std::vector<CImg8> screenshots;
    int matched_frames;

    protected:
        void load_screenshots();
        void save_debug_frames(const CImg8 &sreenshot, const CImg8 &frame, int frame_count);

    public:
        int get_score() { 
            return matched_frames;
        }

        virtual void process_frame(const CImg8 &frame, int frame_count);

        search_video_worker(const search_context &sc) : sc(sc), matched_frames(0) {
            load_screenshots();
        }
};

void search_video_worker::save_debug_frames(const CImg8 &screenshot, const CImg8 &frame, int frame_count) {
    std::stringstream ss;
    ss << frame_count << "-frame.png";
    frame.save(ss.str().c_str());
    ss.str("");

    ss << frame_count << "-screenshot.png";
    screenshot.save(ss.str().c_str());
}

void search_video_worker::process_frame(const CImg8 &frame, int frame_count) {
    for(std::vector<CImg8>::const_iterator it = screenshots.begin();
        it != screenshots.end();
        it++) {
        
        int diff = regioned_diff(*it, frame, frame.width(), frame.height(), sc.compare_limit);
        if(diff >= 0 && diff <= sc.match_limit) {
            matched_frames+=1;
            if(sc.debug_matches) {
                std::cerr << "frames matching " << frame_count << " with diff " << diff << std::endl;
                save_debug_frames(*it, frame, frame_count);
            }
            break;
        }
    }
}

void search_video_worker::load_screenshots() {
    // load the screenshots - and filter them
    for(std::vector<std::string>::const_iterator it = sc.screenshot_paths.begin();
        it != sc.screenshot_paths.end();
        it++) {

        CImg8 new_image;
        new_image.load((*it).c_str());

        new_image.resize(sc.width, sc.height);

        // see if this image passes a histogram test
        if(get_unique_colors(new_image) >= sc.screenshot_limit) {
            screenshots.push_back(new_image);
        }
    }

    std::cerr << "loaded " << sc.screenshot_paths.size() << " screenshots and " << screenshots.size() << " passed the histogram test" << std::endl;
}

int main(int argc, char** argv) {
    search_context sc;

    sc.parse_command_line(argc, argv);

    search_video_worker worker(sc);
    video_processor processor;

    processor.iterate(
        worker,
        sc.video_path,
        0,
        0,
        0,
        sc.width,
        sc.height
    );
    std::cout << worker.get_score();
}
