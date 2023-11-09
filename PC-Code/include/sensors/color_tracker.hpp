#ifndef SPHERO_ROBOT_COLOR_TRACKER_HPP
#define SPHERO_ROBOT_COLOR_TRACKER_HPP

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

// kode tatt fra python og ish direkte oversatt til C++.
//TODO: Fjerne namespace - legge til kommentarer - legge til i Cmake - Dette bør være en .hpp fil
//Maybe change function to not open camera but take in frams? easier to implement in other places
//Stuff that needs to be changed:
//remove opening of the camera
//replace cap>> image, just use image
//NOTE! This will remove any tracking or 3d stuff, since we only get one frame at a time, so we cannot compare it.
//we could feed in previous frames, but that will make calling the function for the first time a bit more complicated.
//(and the 3d stuff is not necessary if we use LIDAR)

float colorTracker(cv::Mat image) {
    int min_contour_area = 500; //change to filter noise

    // Define the range of the color we are looking for
    cv::Scalar lower_bound(13, 175, 50);
    cv::Scalar upper_bound(23, 255, 255);

    cv::Mat hsv, mask, segmented, gray, edges;
    std::vector<std::vector<cv::Point>> contours;

    if (image.empty()) {
        std::cout << "Error: Couldn't read a frame." << std::endl;
        return 0;
    }

    // Turn image into hsv
    cvtColor(image, hsv, cv::COLOR_BGR2HSV);

    // Find color in predefined range
    inRange(hsv, lower_bound, upper_bound, mask);
    bitwise_and(image, image, segmented, mask);

    // Find contours
    findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double weightedSumX = 0.0, weightedSumY = 0.0, totalArea = 0.0;

    // Go through all contours found
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > min_contour_area) {
            cv::Moments M = moments(contour);
            cv::Point2f centroid;

            if (M.m00 != 0) {
                centroid = cv::Point2f(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));

                // Add to the weighted sum and total area
                weightedSumX += centroid.x * area;
                weightedSumY += centroid.y * area;
                totalArea += area;
            }

            //Draw a circle on each contour's centroid and display the size
            //circle(image, centroid, 5, Scalar(255, 0, 0), -1);
            //string label = "Size: " + to_string(area) + " px";
            //putText(image, label, centroid, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1, LINE_AA);
        }
    }

    // Avoid division by zero
    if (totalArea > 0) {
        cv::Point2f overallCentroid = cv::Point2f(static_cast<float>(weightedSumX / totalArea),
                                          static_cast<float>(weightedSumY / totalArea));

        // Draw a single circle at the weighted centroid of all contours
        circle(image, overallCentroid, 10, cv::Scalar(0, 0, 255), -1);

        float imageMiddleX = image.cols / 2.0f;
        float difference = overallCentroid.x - imageMiddleX;

        // Normalize the difference to range -0.5 and 0.5
        float normalizedDifference = difference / image.cols;
        std::cout << "Normalized difference between centroid x-position and image middle: " << normalizedDifference << std::endl;

        return normalizedDifference;
    }
    else {
        return 0;
    }

    // Display the resulting image
    imshow("Contours", image);



}


#endif//SPHERO_ROBOT_COLOR_TRACKER_HPP
