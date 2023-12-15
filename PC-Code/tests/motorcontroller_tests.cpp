#include <iostream>
#include <thread>
#include "catch2/catch_test_macros.hpp"
#include "control/motorcontroller.hpp"

TEST_CASE("Follow Me", "[MotorControlling1]") {
    SECTION("Valid Object Detected") {
        float diff = 0.1f;
        float max_speed = 1.0f;
        int distance = 200;
        bool is_valid = true;
        TankSteering result = FollowMe(diff, distance, is_valid, max_speed);
        // these values can be changed to simulate the logic
        int expected_left_belt = 26;
        int expected_right_belt = 21;

        REQUIRE(result.left_belt_ == expected_left_belt);
        REQUIRE(result.right_belt_ == expected_right_belt);
        std::cout << "Success!" << std::endl;
    }

}

TEST_CASE("Normalize Belts", "[MotorControlling2]") {
    SECTION("Proportional Control") {
        float left_belt_float = 1.0f, right_belt_float = 0.5f;
        float max_speed = 1.0f, input_strength = 1.0f, turn_speed = 1.0f;
        TankSteering result = NormalizeBelts(left_belt_float, right_belt_float, max_speed, input_strength, turn_speed);
        REQUIRE(result.left_belt_ == 255);
        REQUIRE(result.right_belt_ == 128);
        std::cout << "Test success" << std::endl;
    }
    SECTION("Value within bounds") {
        float value = 400;
        float lower_bound = 100;
        float upper_bound = 500;
        float expected = 0.5; // Expected output for this input
        float tolerance = 0.6; // Tolerance for floating-point comparison

        float actual = AutoStop(value, lower_bound, upper_bound);
        std::cout << value << std::endl;
        std::cout << lower_bound << std::endl;
        std::cout << upper_bound << std::endl;
        std::cout << (value - lower_bound) / (upper_bound - lower_bound) << std::endl;

        REQUIRE(std::abs(actual - expected) < tolerance);
        std::cout << "Test success!" << std::endl;
    }

    // Here we can add other tests with other parameters
}

TEST_CASE("Auto Stop", "[MotorControlling3]") {
    SECTION("Value within bounds") {
        int value = 300;
        int lower_bound = 100;
        int upper_bound = 500;
        float expected = 0.5f; // Expected output for this input
        float tolerance = 0.01f; // Tolerance for floating-point comparison

        float actual = AutoStop(value, lower_bound, upper_bound);
        REQUIRE(std::abs(actual - expected) < tolerance);
        std::cout << "Test success!" << std::endl;
    }

    // Here we can add other tests with other parameters
}