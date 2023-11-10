#include <iostream>
#include <SDL.h>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <cmath>

using boost::asio::ip::udp;

class UDPHandler {
public:
    UDPHandler()
        : io_service_(), socket_(io_service_), remote_endpoint_(boost::asio::ip::address::from_string("10.25.46.49"), 6001) {
        socket_.open(udp::v4());
        sendMessage("Hello"); // Send initial message upon creation
    }

    void sendMessage(const std::string& message) {
        socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
    }

    cv::Mat receiveFrame() {
        boost::array<char, 65536> recv_buf;
        size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint_);
        std::string encoded_data(recv_buf.begin(), recv_buf.begin() + len);
        std::string decoded_data = base64_decode(encoded_data);
        std::vector<uchar> buf(decoded_data.begin(), decoded_data.end());
        return cv::imdecode(buf, cv::IMREAD_COLOR);
    }

private:
    boost::asio::io_service io_service_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;

    std::string base64_decode(const std::string &in) {
        std::string out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

        int val = 0, valb = -8;
        for (uchar c: in) {
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
};

class TankSteering {
public:
    int leftBelt;
    int rightBelt;

    static TankSteering calculate(const Uint8 *keyboardState, SDL_Joystick *joystick);
};

class NetworkHandler {
public:
    void handle_network(SDL_Joystick *joystick);
};

class VideoHandler {
public:
    void handle_video(bool enableColorTracking);

private:
    void colorTracking();

    cv::Mat receiveFrame(udp::socket &socket, udp::endpoint &remote_endpoint);
};

TankSteering getTankSteering(const Uint8 *keyboardState, SDL_Joystick *joystick) {

    float leftBeltFloat = 0.0, rightBeltFloat = 0.0;

    //Checking keypresses
    bool isUpPressed = keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP];
    bool isDownPressed = keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN];
    bool isLeftPressed = keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT];
    bool isRightPressed = keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT];

    //Tune this variable to tune steering strength while driving
    float steer = 0.5;
    //Max speed of the robot
    float maxSpeed = 0.4;
    //Tilt of the joystick
    float inputStrength = 1;

    //KBM inputs
    if (isUpPressed || isDownPressed || isLeftPressed || isRightPressed) {

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

        inputStrength = sqrt(x * x + y * y) / 32768;

        if (inputStrength > 1) {
            inputStrength = 1;
        }

        if (inputStrength > 0.2) {
            leftBeltFloat = -y + steer * x;
            rightBeltFloat = -y - steer * x;
        } else {
            leftBeltFloat = 0;
            rightBeltFloat = 0;
        }
    }

    // Normalize
    if ((fabs(leftBeltFloat) > fabs(rightBeltFloat)) and (round(leftBeltFloat) != 0)) {
        rightBeltFloat = rightBeltFloat / fabs(leftBeltFloat);
        leftBeltFloat = leftBeltFloat / fabs(leftBeltFloat);
    } else if (round(rightBeltFloat) != 0) {
        leftBeltFloat = leftBeltFloat / fabs(rightBeltFloat);
        rightBeltFloat = rightBeltFloat / fabs(rightBeltFloat);
    }

    TankSteering steering;
    steering.leftBelt = round(leftBeltFloat * 255 * maxSpeed * inputStrength);
    steering.rightBelt = round(rightBeltFloat * 255 * maxSpeed * inputStrength);

    return steering;
}

void NetworkHandler::handle_network(SDL_Joystick *joystick) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6000);
    boost::asio::ip::tcp::socket socket(io_service);

    try {
        socket.connect(endpoint);
    } catch (boost::system::system_error &e) {
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
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
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

void VideoHandler::handle_video(bool enableColorTracking) {
    if (enableColorTracking) {
        colorTracking();
    } else {
        try {
            std::cout << "Starting video thread" << std::endl;
            UDPHandler udpHandler; // Using the new UDPHandler class

            while (true) {
                std::cout << "Starting video loop" << std::endl;
                cv::Mat frame = udpHandler.receiveFrame(); // Receiving frame using UDPHandler

                if (!frame.empty()) {
                    std::cout << "Received frame of size: " << frame.size() << std::endl;
                    cv::imshow("Received Video", frame);
                } else {
                    std::cerr << "Received empty frame" << std::endl;
                }

                if (cv::waitKey(10) == 'q') {
                    break;
                }
            }
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void VideoHandler::colorTracking() {
    try {
        UDPHandler udpHandler;

        cv::Scalar lower_bound(13, 175, 50);
        cv::Scalar upper_bound(23, 255, 255);
        cv::Mat hsv, mask, segmented;
        std::vector<std::vector<cv::Point>> contours;
        int min_contour_area = 500;

        while (true) {
            std::cout << "Starting color tracking" << std::endl;
            cv::Mat image = udpHandler.receiveFrame();
            if (image.empty()) {
                std::cerr << "Could not decode frame!" << std::endl;
                continue;
            }

            // Color detection and contour finding
            cvtColor(image, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, lower_bound, upper_bound, mask);
            bitwise_and(image, image, segmented, mask);
            findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            double weightedSumX = 0.0, weightedSumY = 0.0, totalArea = 0.0;

            for (const auto &contour: contours) {
                double area = contourArea(contour);
                if (area > min_contour_area) {
                    cv::Moments M = moments(contour);
                    if (M.m00 != 0) {
                        cv::Point2f centroid(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));
                        weightedSumX += centroid.x * area;
                        weightedSumY += centroid.y * area;
                        totalArea += area;

                        circle(image, centroid, 5, cv::Scalar(255, 0, 0), -1);
                        std::string label = "Size: " + std::to_string(area) + " px";
                        putText(image, label, centroid, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1,
                                cv::LINE_AA);
                    }
                }
            }

            if (totalArea > 0) {
                cv::Point2f overallCentroid(static_cast<float>(weightedSumX / totalArea),
                                            static_cast<float>(weightedSumY / totalArea));
                circle(image, overallCentroid, 10, cv::Scalar(0, 0, 255), -1);
            }

            imshow("Color Tracking", image); //"image" for raw and "segmented" for only the color

            if (cv::waitKey(10) == 'q') {
                break;
            }
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

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

    bool enableColorTracking = false;
    std::cout << "Enter 1 for Sphero control, 2 for color tracking: ";
    int mode;
    std::cin >> mode;
    if (mode == 2) {
        enableColorTracking = true;
    }

    // Networking and video threads now need to consider the user's choice
    std::thread network_thread;
    if (!enableColorTracking) {
        network_thread = std::thread([&]() {
            NetworkHandler networkHandler;
            networkHandler.handle_network(joystick);
        });
    }

    std::thread video_thread([&]() {
        VideoHandler videoHandler;
        videoHandler.handle_video(enableColorTracking);
    });

    // Main loop now only handles window events
    bool runLoop = true;
    while (runLoop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                runLoop = false;
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

    // Ensure threads are joined before exiting
    if (network_thread.joinable()) {
        network_thread.join();
    }
    video_thread.join();

    return 0;
}