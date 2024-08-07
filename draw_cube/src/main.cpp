#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"
#include "fdcl_common.hpp"

bool isStringValid(const std::string& str);
void saveCapturedStrings(const std::vector<std::string>& captured_strings, const std::string& filename);
bool isStringInVector(const std::vector<std::string>& vec, const std::string& str);
void cmp(const std::string& filename);

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

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionary_id));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);
    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    int frame_width = in_video.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = in_video.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fps = 30;
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter video("out.avi", fourcc, fps, cv::Size(frame_width, frame_height), true);

    std::map<int, std::string> id_to_string = {
        {0, "1"}, {1, "2"}, {2, "3"}, {3, "4"}, {4, "5"}, {5, "6"}, {6, "7"}, {7, "8"}, {8, "9"}, {9, "10"},
        {11, "new"}, {12, "array"}, {13, "="}, {14, "insert"}, {15, "("}, {16, ")"}, {17, ";"}, {18, "delete"}, {19, "resultado"}
    };

    std::vector<int> myArray;
    cv::Vec3d rvec19, tvec19;
    bool marker19_found = false;

    std::vector<std::string> captured_strings;

    while (in_video.grab()) {
        in_video.retrieve(image);
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        std::string detected_string;

        if (ids.size() > 0) {
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m, camera_matrix, dist_coeffs, rvecs, tvecs);

            std::vector<std::pair<int, std::vector<cv::Point2f>>> sorted_markers;
            for (size_t i = 0; i < ids.size(); ++i) {
                sorted_markers.push_back(std::make_pair(ids[i], corners[i]));
            }
            std::sort(sorted_markers.begin(), sorted_markers.end(), [](const std::pair<int, std::vector<cv::Point2f>>& a, const std::pair<int, std::vector<cv::Point2f>>& b) {
                return a.second[0].x < b.second[0].x;
            });

            for (const auto& marker : sorted_markers) {
                int id = marker.first;
                if (id == 19) {
                    rvec19 = rvecs[&marker - &sorted_markers[0]];
                    tvec19 = tvecs[&marker - &sorted_markers[0]];
                    marker19_found = true;
                    continue;
                }
                if (id_to_string.find(id) != id_to_string.end()) {
                    detected_string += id_to_string[id] + " ";
                } else {
                    detected_string += "error ";
                }
            }

            if (!detected_string.empty()) {
                detected_string.pop_back();
            }

            std::vector<Token> tokens = LexicalAnalyzer::analyze(detected_string);
            std::cout << "Lexical Tokens:" << std::endl;
            for (const auto& token : tokens) {
                std::cout << "{ lexeme: \"" << token.lexeme << "\", type: " << token.type << " }" << std::endl;
            }

            bool syntax_valid = SyntaxAnalyzer::parse(tokens);

            if (syntax_valid && marker19_found && !detected_string.empty() && !isStringInVector(captured_strings, detected_string) && SemanticAnalyzer::analyze(tokens)) {
                captured_strings.push_back(detected_string);
            }

            std::cout << "Detected string: " << detected_string << std::endl;

            if (syntax_valid && SemanticAnalyzer::analyze(tokens)) {
                std::cout << "Semantic Analysis Passed" << std::endl;
                CodeGenerator::generate(tokens);
                if (tokens[0].lexeme == "new" && tokens[1].lexeme == "array") {
                    int array_size = std::stoi(tokens[3].lexeme);
                    myArray = std::vector<int>(array_size, 0);
                } else if (tokens[0].lexeme == "insert") {
                    int index = std::stoi(tokens[2].lexeme);
                    int value = std::stoi(tokens[5].lexeme);
                    if (index >= 0 && index < myArray.size()) {
                        myArray[index] = value;
                    }
                } else if (tokens[0].lexeme == "delete") {
                    int index = std::stoi(tokens[2].lexeme);
                    if (index >= 0 && index < myArray.size()) {
                        myArray[index] = 0;
                    }
                }

                if (marker19_found) {
                    std::cout << "..." << std::endl;
                }
            } else if (syntax_valid) {
                std::cout << "Semantic Analysis Failed" << std::endl;
            } else {
                std::cout << "Syntax Analysis Failed" << std::endl;
            }
        }

        video.write(image_copy);
        cv::imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27) {
            break;
        } else if (key == 'c' || key == 'C') {
            saveCapturedStrings(captured_strings, "captured_strings.txt");
            std::cout << "Captured strings saved to captured_strings.txt" << std::endl;
            cmp("captured_strings.txt");
        }
    }

    in_video.release();

    return 0;
}

bool isStringValid(const std::string& str) {
    std::vector<Token> tokens = LexicalAnalyzer::analyze(str);
    bool syntax_valid = SyntaxAnalyzer::parse(tokens);
    return syntax_valid && SemanticAnalyzer::analyze(tokens);
}

bool isStringInVector(const std::vector<std::string>& vec, const std::string& str) {
    return std::find(vec.begin(), vec.end(), str) != vec.end();
}

void saveCapturedStrings(const std::vector<std::string>& captured_strings, const std::string& filename) {
    std::ofstream output_file(filename);
    if (!output_file.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (const auto& str : captured_strings) {
        output_file << str << std::endl;
    }

    output_file.close();
}

void generateObjectCode(const std::vector<int>& array, const std::string& outputFile) {
    std::string outputPath = "../../array/src/" + outputFile;
    
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Could not open the output file.\n";
        return;
    }
    out << R"cpp(
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

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionary_id));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);
    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    int frame_width = in_video.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = in_video.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fps = 30;
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter video("out.avi", fourcc, fps, cv::Size(frame_width, frame_height), true);

    std::map<int, std::string> id_to_string = {
        {11, "new"}, {12, "array"}, {13, "="}, {14, "insert"}, {15, "("}, {16, ")"}, {17, ";"}, {18, "delete"}, {19, "resultado"}
    };

    while (in_video.grab()) {
        in_video.retrieve(image);
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        std::string detected_string;

        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);

            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m, camera_matrix, dist_coeffs, rvecs, tvecs);

            for (int i = 0; i < ids.size(); i++) {
                if (ids[i] == 19) {
                    std::vector<int> values = {)cpp";

    for (size_t i = 0; i < array.size(); ++i) {
        out << array[i];
        if (i != array.size() - 1) {
            out << ", ";
        }
    }

    out << R"cpp(};
                    drawMultipleCubesWireframe(image_copy, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], marker_length_m, values);
                }

                if (ids[i] <= 10) {
                    detected_string += std::to_string(ids[i]) + " ";
                } else if (id_to_string.find(ids[i]) != id_to_string.end()) {
                    detected_string += id_to_string[ids[i]] + " ";
                } else {
                    detected_string += "error ";
                }
            }

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

    for (int n = 0; n < num_cubes; ++n) {
        float offset = n * l;

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

        for (int i = 0; i < 4; i++) {
            cv::line(image, image_points[i], image_points[(i + 1) % 4], cv::Scalar(255, 0, 0), 3);
            cv::line(image, image_points[i + 4], image_points[((i + 1) % 4) + 4], cv::Scalar(255, 0, 0), 3);
            cv::line(image, image_points[i], image_points[i + 4], cv::Scalar(255, 0, 0), 3);
        }

        // Calculate the center of the cube
        cv::Point2f center(0, 0);
        for (int i = 0; i < 8; i++) {
            center += image_points[i];
        }
        center *= (1.0 / 8.0);

        // Draw the value at the center of the cube
        std::string value_text = std::to_string(values[n]);
        int baseline = 0;
        cv::Size text_size = cv::getTextSize(value_text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, &baseline);
        cv::Point text_origin(center.x - text_size.width / 2, center.y + text_size.height / 2);
        cv::putText(image, value_text, text_origin, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
    }
}
)cpp";
    out.close();
}

void cmp(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Could not open the input file.\n";
        return;
    }

    std::vector<int> array;
    std::string line;

    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "new") {
            std::cerr << "Processing 'new' command.\n";
            std::string array_str, equals;
            int size;
            iss >> array_str >> equals >> size >> equals;

            if (size > 0) {
                array.resize(size, 0);
                std::cerr << "Created array of size " << size << ".\n";
            } else {
                std::cerr << "Invalid array size.\n";
                return;
            }
        } else if (command == "insert") {
            std::cerr << "Processing 'insert' command.\n";
            std::string token;
            char parenthesis1, parenthesis2, equals;
            int index, value;
            iss >> parenthesis1 >> index >> parenthesis2 >> equals >> value >> token;

            if (index >= 0 && index < array.size()) {
                array[index] = value;
                std::cerr << "Inserted " << value << " at position " << index << ".\n";
            } else {
                std::cerr << "Invalid index for insert.\n";
                return;
            }
        } else if (command == "delete") {
            std::cerr << "Processing 'delete' command.\n";
            std::string token;
            char parenthesis1, parenthesis2;
            int index;
            iss >> parenthesis1 >> index >> parenthesis2 >> token;

            if (index >= 0 && index < array.size()) {
                array[index] = 0;
                std::cerr << "Deleted element at position " << index << ".\n";
            } else {
                std::cerr << "Invalid index for delete.\n";
                return;
            }
        } else {
            std::cerr << "Unknown command: " << command << "\n";
            return;
        }
    }

    inFile.close();
    generateObjectCode(array, "main.cpp");
    std::cout << "Generated code for the array operations.\n";
}
