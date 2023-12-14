#define CATCH_CONFIG_MAIN
#include "communications/mock_server.hpp"
#include "communications/client.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "catch2/catch_test_macros.hpp"


// Assuming the definitions of TankSteering and HandleControlling() are accessible
extern void HandleControlling(TankSteering& steer, std::mutex& steer_mutex);
extern void handle_video(cv::Mat& frame, std::mutex& frame_mutex);

// Mock data for testing
const std::string mockTCPData = "Mock TCP Command";
const cv::Mat mockImage = cv::Mat::zeros(100, 100, CV_8UC3);  // A simple black image for testing

TEST_CASE("TCP Communication Test") {
    MockServer server(6000, 6001);

    // Set up TCP handler to capture data sent from the client
    server.setTCPHandler([](const std::string& data) {
        REQUIRE(data == mockTCPData);
    });

    // Simulate client behavior
    TankSteering steer;
    std::mutex steer_mutex;
    std::thread clientThread([&steer, &steer_mutex]() {
        HandleControlling(steer, steer_mutex);
    });

    // Allow time for server to set up and client to connect
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Send mock data to the client (normally done within HandleControlling)
    {
        std::lock_guard<std::mutex> lock(steer_mutex);
        steer.left_belt_ = 1;  // Mock values
        steer.right_belt_ = 1;
    }

    clientThread.join();
}

TEST_CASE("UDP Communication Test") {
    MockServer server(6000, 6001);

    // Set up UDP handler to capture data sent from the client
    server.setUDPHandler([](const cv::Mat& image) {
        REQUIRE(!image.empty());
    });

    // Simulate client behavior
    std::mutex frame_mutex;
    std::thread clientThread([&frame_mutex]() {
        cv::Mat frame;
        handle_video(frame, frame_mutex);
    });

    // Allow time for server to set up and client to connect
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Send mock data to the client (normally done within handle_video)
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        server.sendMockUDPData(mockImage);
    }

    clientThread.join();
}
