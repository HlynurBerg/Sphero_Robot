#include <SDL.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "communications/client.hpp"
#include "communications/thread_safe_queue.hpp"
#include "communications/websocket.hpp"
#include "control/motorcontroller.hpp"
#include "sensors/sensor_processing.hpp"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Tank Steering", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Joystick *joystick = nullptr;
    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0); // Open the first available joystick
    }

    // Create hardcoded data receiver and udp handler for easier changing between home and school network
    TCPHandler data_receiver("10.25.46.49", 6003);
    UDPHandler udp_handler("10.25.46.49", 6001);

    // Create thread safe queues for video frames
    ThreadSafeQueue<std::shared_ptr<std::string>> frame_queue_for_machine_vision;
    ThreadSafeQueue<std::shared_ptr<std::string>> frame_queue_for_video_thread;

    // Create atomic variables for controlling the robot
    std::atomic<bool> enable_color_tracking(false);
    std::atomic<float> max_speed(0.3f);

    TankSteering steer;
    std::mutex steer_mutex;

    // Creating jthreads as for C++20
    std::jthread steering_thread([&]() {
        TCPHandler::HandleControlling(std::ref(steer), std::ref(steer_mutex));
    });

    // producer_thread receives undecoded base64 video frames and pushes them to the frameQueue
    std::jthread producer_thread([&]() {
        while (true) {
            auto frame = std::make_shared<std::string>(udp_handler.ReceiveBase64Frame());
            frame_queue_for_machine_vision.PushFrame(frame);
            frame_queue_for_video_thread.PushFrame(frame);
        }
    });

    std::pair<float, bool> result;

    std::jthread machine_vision_thread([&]() {
        std::shared_ptr<std::string> base64_frame;
        while (true) {
            frame_queue_for_machine_vision.WaitAndPopFrame(base64_frame);
            std::string decoded_data = udp_handler.Base64Decode(*base64_frame);
            std::vector<uchar> buf(decoded_data.begin(), decoded_data.end());
            cv::Mat frame = cv::imdecode(buf, cv::IMREAD_COLOR);

            // Could be imported from websocket, but for now hardcoded
            cv::Scalar lower_bound(10, 150, 50); // yellow, quite saturated
            cv::Scalar upper_bound(25, 255, 255);
            int min_contour_area = 250;

            // Process the frame with ColorTracker
            result = ColorTracker(frame, lower_bound, upper_bound, min_contour_area);
        }
    });

    // Create an instance of io_context
    net::io_context ioc;

    // Set up the WebSocket server
    tcp::endpoint endpoint(tcp::v4(), 8080);
    WebSocketServer wsServer(ioc, endpoint, data_receiver, enable_color_tracking, max_speed);

    std::jthread video_thread([&]() {
        std::shared_ptr<std::string> base64_frame;
        while (true) {
            frame_queue_for_video_thread.WaitAndPopFrame(base64_frame);
            if (!base64_frame->empty()) {
                wsServer.BroadcastVideoFrame(*base64_frame);
            }
        }
    });

    // Run the io_context in a separate thread
    std::jthread websocket_thread([&ioc](){ ioc.run(); });

    // Main loop now only handles window events
    bool run_loop = true;
    while (run_loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run_loop = false;
            }
        }

        data_receiver.UpdateData(); // Update data from server

        double battery_percentage = data_receiver.GetBatteryPercentage();
        double distance_mm = data_receiver.GetDistanceMm();
        double speed_y = data_receiver.GetSpeedY();

        const Uint8 *keyboard_state = SDL_GetKeyboardState(nullptr);

        //Steering the RVR
        if (enable_color_tracking) {

            float diff = result.first;
            bool is_valid = result.second;
            TankSteering temp_steer;

            std::cout << "diff: " << diff << std::endl;
            temp_steer = FollowMe(diff, distance_mm, is_valid, max_speed);
            {
                std::lock_guard<std::mutex> lock(steer_mutex);
                steer = temp_steer;
            }

            std::cout << "left_belt: " << steer.left_belt_ << " right_belt: " << steer.right_belt_ << std::endl;
        }

        else {
            TankSteering temp_steer = GetTankSteering(keyboard_state, joystick, distance_mm, max_speed);
            {
                std::lock_guard<std::mutex> lock(steer_mutex);
                steer = temp_steer;
            }

        }
        SDL_Delay(10);

    }

    // Cleanup
    if (joystick) {
        SDL_JoystickClose(joystick);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}