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
class NetworkHandler {
public:
    void handle_controlling(SDL_Joystick *joystick);
};

class VideoHandler {
public:
    void handle_video(bool enableColorTracking);

private:
    void colorTracking();
};
/*
void NetworkHandler::handle_autonome() {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6000);
    boost::asio::ip::tcp::socket socket(io_service);

    //TODO: Implement the Difference from the colortracking and make it steer the robot to keep the ball in the center of the screen
}
*/



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

                // Calculate the horizontal difference from the center of the screen
                float screenCenterX = segmented.cols / 2.0f;
                float difference = overallCentroid.x - screenCenterX;

                std::cout << "Difference: " << difference << std::endl;

                //TODO: Implement an autonome steering function to keep the ball in the center of the screen

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
            networkHandler.handle_controlling(joystick);
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