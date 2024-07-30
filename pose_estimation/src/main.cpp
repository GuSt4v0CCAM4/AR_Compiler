#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <cstdlib>

#include "fdcl_common.hpp"


int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, fdcl::keys);

    const char* about = "Pose estimation of ArUco marker images";

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
    float marker_length_m = parser.get<float>("l");
    int wait_time = 10;

    if (marker_length_m <= 0) {
        std::cerr << "Marker length must be a positive value in meter\n";
        return 1;
    }

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;

    std::ostringstream vector_to_marker;

    // Create the dictionary from the same dictionary the marker was generated.
    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary( \
        cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionary_id));


    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);
    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;


    while (in_video.grab())
    {
        in_video.retrieve(image);
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        // if at least one marker detected
        if (ids.size() > 0)
        {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);

            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m,
                    camera_matrix, dist_coeffs, rvecs, tvecs);
                    
            std::cout << "Translation: " << tvecs[0]
                << "\tRotation: " << rvecs[0] << "\n";
            
            // Draw axis for each marker
            for(int i=0; i < ids.size(); i++)
            {
                cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs,
                        rvecs[i], tvecs[i], 0.1);

                // This section is going to print the data for the first the 
                // detected marker. If you have more than a single marker, it is 
                // recommended to change the below section so that either you
                // only print the data for a specific marker, or you print the
                // data for each marker separately.
                drawText(image_copy, "x", tvecs[0](0), cv::Point(10, 30));
                drawText(image_copy, "y", tvecs[0](1), cv::Point(10, 50));
                drawText(image_copy, "z", tvecs[0](2), cv::Point(10, 70));
            }
        }

        imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27) {
            break;
        }
    }

    in_video.release();

    return 0;
}
