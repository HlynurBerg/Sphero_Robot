//
// Created by jorge on 10/16/2023.
//
#ifndef TESTABLE_NETWORKING_TCP_CLIENT_HPP
#define TESTABLE_NETWORKING_TCP_CLIENT_HPP

#include <boost/asio.hpp>
#include "boost/array.hpp"
#include <array>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "SDL.h"
#include <opencv2/opencv.hpp>

#include "comms/communicationhandler.hpp"


using boost::asio::ip::udp;

// Base64 decoding function - maybe replace with a library later
std::string base64_decode(const std::string &in);

class TankSteering {
public:
    int leftBelt;
    int rightBelt;

    static TankSteering calculate(const Uint8* keyboardState, SDL_Joystick* joystick);
};

class NetworkHandler {
public:
    void handle_network(SDL_Joystick* joystick);
};

class VideoHandler {
public:
    void handle_video();
};

// Definitions

std::string base64_decode(const std::string &in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

TankSteering normalizeBelts(float leftBeltFloat, float rightBeltFloat, float maxSpeed, float inputStrength, float turnSpeed) {
    if ((fabs(leftBeltFloat) > fabs(rightBeltFloat)) and (round(leftBeltFloat) != 0)) {
        rightBeltFloat = rightBeltFloat/fabs(leftBeltFloat);
        leftBeltFloat = leftBeltFloat/fabs(leftBeltFloat);
    }
    else if (round(rightBeltFloat) != 0) {
        leftBeltFloat = leftBeltFloat/fabs(rightBeltFloat);
        rightBeltFloat = rightBeltFloat/fabs(rightBeltFloat);
    }

    TankSteering normalized;
    if (leftBeltFloat == -rightBeltFloat) {
        normalized.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength * turnSpeed);
        normalized.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength * turnSpeed);
    }
    else {
        normalized.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength);
        normalized.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength);
    }
    return normalized;
}


TankSteering getTankSteering(const Uint8* keyboardState, SDL_Joystick* joystick) {

    float leftBeltFloat = 0.0, rightBeltFloat = 0.0;

    //Checking keypresses
    bool isUpPressed = keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP];
    bool isDownPressed = keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN];
    bool isLeftPressed = keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT];
    bool isRightPressed = keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT];

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;
    //Max speed of the robot
    float maxSpeed = 0.3;
    //Tilt of the joystick
    float inputStrength = 1;
    //Speed of rotation
    float turnSpeed = 0.5;

    //KBM inputs
    if(isUpPressed || isDownPressed || isLeftPressed || isRightPressed) {

        // Adding up inputs
        if (isUpPressed) {
            leftBeltFloat++;
            rightBeltFloat++;
        }
        if (isDownPressed) {
            leftBeltFloat--;
            rightBeltFloat--;
        }
        if (isLeftPressed) {
            leftBeltFloat = leftBeltFloat - steer;
            rightBeltFloat = rightBeltFloat + steer;
        }
        if (isRightPressed) {
            leftBeltFloat = leftBeltFloat + steer;
            rightBeltFloat = rightBeltFloat - steer;
        }
    }

    //Joystick input
    else {
        int x = SDL_JoystickGetAxis(joystick, 0);
        int y = SDL_JoystickGetAxis(joystick, 1);

        inputStrength = sqrt(x * x + y * y)/32768;

        if (inputStrength > 1) {
            inputStrength = 1;
        }

        if (inputStrength > 0.2) {
            leftBeltFloat = -y + steer*x;
            rightBeltFloat = -y - steer*x;
        }
        else {
            leftBeltFloat = 0;
            rightBeltFloat = 0;
        }
    }
    return normalizeBelts(leftBeltFloat, rightBeltFloat, maxSpeed, inputStrength, turnSpeed);
}

TankSteering followMe(float difference, float forwards) {
    float inputStrength = 1;
    float maxSpeed = 0.3;
    float steer = 0.5;
    float turnSpeed = 1;
    if(forwards < 0.1){
        turnSpeed = 2*abs(difference);
    }
    float leftBeltFloat = forwards + steer*difference;
    float rightBeltFloat = forwards - steer*difference;

    return normalizeBelts(leftBeltFloat, rightBeltFloat, maxSpeed, inputStrength, turnSpeed);
}


void NetworkHandler::handle_network(SDL_Joystick* joystick) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6000);
    boost::asio::ip::tcp::socket socket(io_service);

    try {
        socket.connect(endpoint);
    } catch (boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) { // Process SDL events
            if (e.type == SDL_QUIT) {
                // Handle quit event if necessary
                return;
            }
            // You can add more event handling as needed here
        }

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        TankSteering result = getTankSteering(keyboardState, joystick);

        std::string command = std::to_string(result.leftBelt) + ", " + std::to_string(result.rightBelt) + "\n";
        boost::system::error_code error;
        socket.write_some(boost::asio::buffer(command), error);

        std::cout << "Sending Command: " << command << std::endl;

        if (error) {
            std::cerr << "Error while sending data: " << error.message() << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void VideoHandler::handle_video() {
    try {
        boost::asio::io_service io_service;
        udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
        udp::endpoint remote_endpoint = udp::endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6001);

        std::string init_message = "Hello";
        socket.send_to(boost::asio::buffer(init_message), remote_endpoint);

        while (true) {
            boost::array<char, 65536> recv_buf{};
            size_t len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
            std::string encoded_data(recv_buf.begin(), recv_buf.begin() + len);
            std::string decoded_data = base64_decode(encoded_data);
            std::vector<uchar> buf(decoded_data.begin(), decoded_data.end());
            cv::Mat frame = cv::imdecode(buf, cv::IMREAD_COLOR);

            if (frame.empty()) {
                std::cerr << "Could not decode frame!" << std::endl;
                continue;
            }

            imshow("Received Video", frame);

            if (cv::waitKey(30) >= 0) {
                break;
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

#endif//TESTABLE_NETWORKING_TCP_CLIENT_HPP
