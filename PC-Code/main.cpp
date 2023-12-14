#include <SDL.h>
#include <iostream>
#include <thread>
#include <string>
#include "communications/client.hpp"
#include "sensors/sensordata.hpp"
#include "control/motorcontroller.hpp"
#include "communications/websocket.hpp"
#include "communications/thread_safe_queue.hpp"

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
    DataReceiver dataReceiver("10.25.46.49", 6003);
    UDPHandler udpHandler("10.25.46.49", 6001);

    ThreadSafeQueue<std::shared_ptr<std::string>> frameQueueForMachineVision;
    ThreadSafeQueue<std::shared_ptr<std::string>> frameQueueForVideoThread;

    // Creating threads
    TankSteering steer;
    std::mutex steer_mutex;

    std::thread steering_thread([&]() {
        handle_controlling(std::ref(steer), std::ref(steer_mutex));
    });

    // producerThread receives undecoded base64 video frames and pushes them to the frameQueue
    std::thread producerThread([&]() {
        while (true) {
            auto frame = std::make_shared<std::string>(udpHandler.receiveBase64Frame());
            frameQueueForMachineVision.push(frame);
            frameQueueForVideoThread.push(frame);
        }
    });

    std::pair<float, bool> result;

    std::thread machinevision_thread([&]() {
        std::shared_ptr<std::string> base64_frame;
        while (true) {
            frameQueueForMachineVision.wait_and_pop(base64_frame);
            std::string decoded_data = udpHandler.base64_decode(*base64_frame);
            std::vector<uchar> buf(decoded_data.begin(), decoded_data.end());
            cv::Mat frame = cv::imdecode(buf, cv::IMREAD_COLOR);

            // Process the frame with colorTracker
            result = colorTracker(frame);
        }
    });

    bool enableColorTracking = false;

    // Create an instance of io_context
    net::io_context ioc;

    // Set up the WebSocket server
    tcp::endpoint endpoint(tcp::v4(), 8080);
    WebSocketServer wsServer(ioc, endpoint, dataReceiver);

    std::thread videoThread([&]() {
        std::shared_ptr<std::string> base64_frame;
        while (true) {
            frameQueueForVideoThread.wait_and_pop(base64_frame);
            if (!base64_frame->empty()) {
                wsServer.broadcastVideoFrame(*base64_frame);
            }
        }
    });

    // Run the io_context in a separate thread
    std::thread wsThread([&ioc](){ ioc.run(); });

    // Main loop now only handles window events
    bool runLoop = true;
    while (runLoop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                runLoop = false;
            }
        }


        //TODO: add mutex for this data. Should not matter much, but also consider LARS
        dataReceiver.updateData(); // Update data from server
        double battery_percentage = dataReceiver.getBatteryPercentage();
        double distance_mm = dataReceiver.getDistanceMm();
        double speed_y = dataReceiver.getSpeedY();

        //std::cout << "Battery: " << battery_percentage
        //          << "%, Distance: " << distance_mm
        //          << "mm, Speed Y: " << speed_y << " m/s" << std::endl;
        battery_percentage;
        distance_mm;
        speed_y;
        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
        //TODO: remove this when webpage is working. this is better than waiting for user input through terminal
        if (keyboardState[SDL_SCANCODE_Z]){enableColorTracking = true;}
        if (keyboardState[SDL_SCANCODE_X]){enableColorTracking = false;}
        //Steering the RVR
        if (enableColorTracking) {

            float diff = result.first;
            bool isValid = result.second;
            std::cout << "diff: " << diff << std::endl;
            TankSteering tempsteer = followMe(diff, distance_mm, isValid);
            {
                std::lock_guard<std::mutex> lock(steer_mutex);
                steer = tempsteer;
            }
            std::cout << "leftBelt: " << steer.leftBelt << " rightBelt: " << steer.rightBelt << std::endl;
        }

        else {
            TankSteering tempsteer = getTankSteering(keyboardState, joystick, distance_mm);
            {
                std::lock_guard<std::mutex> lock(steer_mutex);
                steer = tempsteer;
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

    //Joining threads before closing program
    if (producerThread.joinable()) {
        producerThread.join();
    }
    if (machinevision_thread.joinable()) {
        machinevision_thread.join();
    }
    if (steering_thread.joinable()) {
        steering_thread.join();
    }
    if (videoThread.joinable()) {
        videoThread.join();
    }
    if (wsThread.joinable()) {
        wsThread.join();
    }
    return 0;
}