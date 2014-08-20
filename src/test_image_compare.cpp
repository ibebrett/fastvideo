#define __STDC_CONSTANT_MACROS

#include <CImg.h>

using namespace cimg_library;

#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include "image_process.hpp"

struct test_image_compare_context {
    int compare_limit;
    int screenshot_limit;
    int match_limit;
    int region_match_limit;
    std::string image1_path;
    std::string image2_path;

    int parse_command_line(int argc, char** argv) {
        namespace po = boost::program_options;
        po::options_description desc("Options");

        desc.add_options()
            ("compare-limit",    po::value<int>()->default_value(6),    "histogram value threshold for comparing screenshots to video")
            ("screenshot-limit", po::value<int>()->default_value(10),   "histogram value threshold to include screenshot in comparisons")
            ("match-limit",      po::value<int>()->default_value(300),  "value to consider a screenshot and frame matching")
            ("region-match-limit", po::value<int>()->default_value(3),  "the number of regions in a diff that must match")
            ("image1",           po::value<std::string>(),              "test image 1")
            ("image2",           po::value<std::string>(),             "test image 2"); 

        po::positional_options_description pos_desc;
        pos_desc.add("image1", 1);
        pos_desc.add("image2", 1);
        po::variables_map vm;

        try {
            po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(),  vm);
            po::notify(vm);
        } catch(po::error& e) {
            std::cerr << "Error during parsing" << std::endl;
            std::cerr << desc;
            return 1;
        }

        this->compare_limit    = vm["compare-limit"].as<int>();
        this->screenshot_limit = vm["screenshot-limit"].as<int>();
        this->match_limit      = vm["match-limit"].as<int>();
        this->region_match_limit = vm["region-match-limit"].as<int>();
        this->image1_path       = vm["image1"].as<std::string>();
        this->image2_path       = vm["image2"].as<std::string>();

        return 0;
    }
};

int main(int argc, char** argv) {
    test_image_compare_context context;

    context.parse_command_line(argc, argv);

    CImg8 image1, image2;
    image1.load(context.image1_path.c_str());
    image2.load(context.image2_path.c_str());

    int num_regions_passed, num_passed_histogram;
    std::cout << "diff " << regioned_diff(image1, image2, image1.width(), image1.height(), context.compare_limit, context.region_match_limit, context.match_limit, num_regions_passed, num_passed_histogram) << std::endl;

    return 0;
}
