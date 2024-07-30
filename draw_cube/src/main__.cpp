
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <map>

#include "fdcl_common.hpp"

void drawMultipleCubesWireframe(cv::InputOutputArray image, cv::InputArray camera_matrix, cv::InputArray dist_coeffs, cv::InputArray rvec, cv::InputArray tvec, float l, const std::vector<int>& values);

int main(int argc, char **argv) {
    cv::CommandLineParser parser(argc, argv, fdcl::keys);

    const char* about = "Draw cube on ArUco marker images";
    auto success = parse_inputs(parser, about);
    if (!success) {
        return 1;
    }

    cv::VideoCapture in_video;
    success = parse_video_in(in_video, parser);
    if (!success) {
        return 1;
    }

    int wait_time = 10;

    int dictionary_id = parser.get<int>("d");
    float marker_length_m = parser.get<float>("l");
    if (marker_length_m <= 0) {
        std::cerr << "Marker length must be a positive value in meter\n";
        return 1;
    }

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;

    // Create the dictionary from the same dictionary the marker was generated.
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionary_id));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);
    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    // Initialize a video writer to save the drawn cube.
    int frame_width = in_video.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = in_video.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fps = 30;
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter video("out.avi", fourcc, fps, cv::Size(frame_width, frame_height), true);

    // Mapping of IDs to strings for 11-19
    std::map<int, std::string> id_to_string = {
        {11, "new"},
        {12, "array"},
        {13, "="},
        {14, "insert"},
        {15, "("},
        {16, ")"},
        {17, ";"},
        {18, "delete"},
        {19, "resultado"}
    };

    while (in_video.grab()) {
        in_video.retrieve(image);
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        std::string detected_string;

        // If at least one marker is detected
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);

            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m, camera_matrix, dist_coeffs, rvecs, tvecs);

            // Draw axis for each marker
            for (int i = 0; i < ids.size(); i++) {
                if (ids[i] == 19) {
                    std::vector<int> values = {0, 6, 5, 4, 0, 0};
                    drawMultipleCubesWireframe(image_copy, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], marker_length_m, values);
                }

                // Mapping IDs to strings and building the detected_string
                if (ids[i] <= 10) {
                    detected_string += std::to_string(ids[i]) + " ";
                } else if (id_to_string.find(ids[i]) != id_to_string.end()) {
                    detected_string += id_to_string[ids[i]] + " ";
                } else {
                    detected_string += "error ";
                }
            }

            // Print the detected string
            std::cout << "Detected string: " << detected_string << std::endl;
        }

        video.write(image_copy);
        cv::imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27) {
            break;
        }
    }

    in_video.release();

    return 0;
}

void drawMultipleCubesWireframe(cv::InputOutputArray image, cv::InputArray camera_matrix, cv::InputArray dist_coeffs, cv::InputArray rvec, cv::InputArray tvec, float l, const std::vector<int>& values) {
    float half_l = l / 2.0;
    int num_cubes = values.size();

    // Draw each cube
    for (int n = 0; n < num_cubes; ++n) {
        float offset = n * l;

        // Project points for the current cube
        std::vector<cv::Point3f> axis_points = {
            cv::Point3f(half_l + offset, half_l, l),
            cv::Point3f(half_l + offset, -half_l, l),
            cv::Point3f(-half_l + offset, -half_l, l),
            cv::Point3f(-half_l + offset, half_l, l),
            cv::Point3f(half_l + offset, half_l, 0),
            cv::Point3f(half_l + offset, -half_l, 0),
            cv::Point3f(-half_l + offset, -half_l, 0),
            cv::Point3f(-half_l + offset, half_l, 0)
        };

        std::vector<cv::Point2f> image_points;
        cv::projectPoints(axis_points, rvec, tvec, camera_matrix, dist_coeffs, image_points);

        // Draw edges of the cube
        for (int i = 0; i < 4; i++) {
            cv::line(image, image_points[i], image_points[(i + 1) % 4], cv::Scalar(255, 0, 0), 3);
            cv::line(image, image_points[i + 4], image_points[((i + 1) % 4) + 4], cv::Scalar(255, 0, 0), 3);
            cv::line(image, image_points[i], image_points[i + 4], cv::Scalar(255, 0, 0), 3);
        }

        // Draw value inside the cube
        cv::putText(image, std::to_string(values[n]), image_points[4] + cv::Point2f(-10, -10), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
    }
}
