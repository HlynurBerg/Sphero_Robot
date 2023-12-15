#include <sensors/sensor_processing.hpp>

std::pair<float, bool> ColorTracker(cv::Mat image, cv::Scalar lower_bound, cv::Scalar upper_bound, int min_contour_area) {
    try {


        bool is_valid = false;
        float difference = 0.0f;
        cv::Mat hsv, mask, segmented;
        std::vector<std::vector<cv::Point>> contours;

        if (image.empty()) {
            std::cerr << "Could not decode frame!" << std::endl;
        }

        // Color and contour detection
        cvtColor(image, hsv, cv::COLOR_BGR2HSV);
        inRange(hsv, lower_bound, upper_bound, mask);
        bitwise_and(image, image, segmented, mask);
        findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        double weighted_sum_x = 0.0, weighted_sum_y = 0.0, total_area = 0.0;

        for (const auto &contour: contours) {
            double area = contourArea(contour);
            if (area > min_contour_area) {
                is_valid = true; // Valid object found
                cv::Moments m_moments = moments(contour);
                if (m_moments.m00 != 0) {
                    cv::Point2f centroid(static_cast<float>(m_moments.m10 / m_moments.m00), static_cast<float>(m_moments.m01 / m_moments.m00));
                    weighted_sum_x += centroid.x * area;
                    weighted_sum_y += centroid.y * area;
                    total_area += area;
                }
            }
        }
        if (total_area > 0) {
            cv::Point2f overall_centroid(static_cast<float>(weighted_sum_x / total_area),
                                        static_cast<float>(weighted_sum_y / total_area));

            /*
            // Uncomment this to draw the overall centroid. Good for testing if it works!
            circle(image, overall_centroid, 10, cv::Scalar(0, 0, 255), -1);
            cv::imshow("Color Tracking", image);
            */

            // Calculate the horizontal difference from the center of the screen
            float screen_center_x = segmented.cols / 2.0f;
            difference = overall_centroid.x - screen_center_x;
            // Normalize to [-1, 1]
            difference = difference / screen_center_x;
        }
        return std::make_pair(difference, is_valid);
    }


    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return std::make_pair(0.0f, false);
    }
}
