#ifndef SPHERO_ROBOT_SENSOR_PROCESSING_HPP
#define SPHERO_ROBOT_SENSOR_PROCESSING_HPP

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

std::pair<float, bool> ColorTracker(cv::Mat image, cv::Scalar lower_bound, cv::Scalar upper_bound, int min_contour_area);

float AutoStop(int value, int lower_bound, int upper_bound);


#endif//SPHERO_ROBOT_SENSOR_PROCESSING_HPP