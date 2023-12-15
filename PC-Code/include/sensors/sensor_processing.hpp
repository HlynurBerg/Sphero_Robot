#ifndef SPHERO_ROBOT_SENSOR_PROCESSING_HPP
#define SPHERO_ROBOT_SENSOR_PROCESSING_HPP

//TODO: remove unused includes
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <vector>
#include <cmath>
#include <nlohmann/json.hpp>

std::pair<float, bool> ColorTracker(cv::Mat image, cv::Scalar lower_bound, cv::Scalar upper_bound, int min_contour_area);

float AutoStop(int value, int lower_bound, int upper_bound);


#endif//SPHERO_ROBOT_SENSOR_PROCESSING_HPP