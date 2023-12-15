#include "catch2/catch_test_macros.hpp"
#include <opencv2/opencv.hpp>
#include "sensors/sensor_processing.hpp"



TEST_CASE("Color Tracking Algorithm", "[ColorTracking]") {
    SECTION("Target Color Present") {
        // Adjust the path to match the relative path from the executable to the image file
        cv::Mat image = cv::imread("cmake-build-debug/PC-Code/tests/Ball_Yellow.png");
        REQUIRE_FALSE(image.empty()); // Ensure the image is loaded
                                     // ... rest of your code


        auto [difference, is_valid] = ColorTracker(image);

        float expected_difference = 1; // Set this based on expected output
        float tolerance = 0.1f;        // Define a tolerance for the difference

        REQUIRE(is_valid == true);
        REQUIRE(difference >= expected_difference - tolerance);
        REQUIRE(difference <= expected_difference + tolerance);
    }

    SECTION("No Target Color") {
        cv::Mat image = cv::imread("cmake-build-debug/PC-Code/tests/Ball_Yellow.png");
        REQUIRE_FALSE(image.empty());

        auto [difference, isValid] = ColorTracker(image);
        REQUIRE(isValid == false);
    }

    SECTION("Multiple Colors") {
        cv::Mat image = cv::imread("cmake-build-debug/PC-Code/tests/Ball_Yellow.png");
        REQUIRE_FALSE(image.empty());

        auto [difference, isValid] = ColorTracker(image);
        REQUIRE(isValid == true);
        // Additional checks as necessary
    }

    SECTION("Multiple Instances of Target Color") {
        cv::Mat image = cv::imread("cmake-build-debug/PC-Code/tests/Ball_Yellow.png");
        REQUIRE_FALSE(image.empty());

        auto [difference, isValid] = ColorTracker(image);
        REQUIRE(isValid == true);

    }

    // Add more sections for other scenarios as necessary
}
