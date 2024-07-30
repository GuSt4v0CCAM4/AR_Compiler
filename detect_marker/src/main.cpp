
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <cstdlib>

#include "fdcl_common.hpp"


int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, fdcl::keys);

    const char* about = "Detect ArUco marker images";
    auto success = parse_inputs(parser, about);
    if (!success) {
        return 1;
    }

    cv::VideoCapture in_video;
    success = parse_video_in(in_video, parser);
    if (!success) {
        return 1;
    }

    int dictionary_id = parser.get<int>("d");
    int wait_time = 10;

    // Create the dictionary from the same dictionary the marker was generated.
    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary( \
        cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionary_id));


    // Process the video
    while (in_video.grab()) {
        cv::Mat image, image_copy;
        in_video.retrieve(image);
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
        }

        imshow("Detected markers", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27) {
            break;
        }
    }

    in_video.release();

    return 0;
}
