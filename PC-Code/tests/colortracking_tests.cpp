#include "catch2/catch_test_macros.hpp"
#include <opencv2/opencv.hpp>
#include "sensors/sensor_processing.hpp"

TEST_CASE("Color Tracking Algorithm", "[ColorTracking]") {
    SECTION("Target Color Present") {
        cv::Mat image = cv::imread("yellow_ball.jpg"); // This image is generated from AI (ChatGPT / Dall-E)
        REQUIRE_FALSE(image.empty()); // Check if the image was loaded correctly

        cv::Scalar lower_bound(10, 100, 150);
        cv::Scalar upper_bound(40, 255, 255);
        int min_contour_area = 1;
        std::pair<float, bool>result = ColorTracker(image, lower_bound, upper_bound, min_contour_area);

        REQUIRE(result.second == true); // Check if the object was detected
        // it doesnt matter what the result value is as long as we get a valid result
    }

    /* optional tests if we have time
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
*/
    // Add more sections for other scenarios as necessary
}
