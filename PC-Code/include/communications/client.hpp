#ifndef SPHERO_ROBOT_CLIENT_HPP
#define SPHERO_ROBOT_CLIENT_HPP

#include <SDL.h> //for User Input. Change when switching to html5 api
#include <boost/asio.hpp> //for communicatoin
#include <opencv2/opencv.hpp> //for image processing
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <thread>
#include <vector>
#include <cmath>


class NetworkHandler {
public:
    void handle_controlling(SDL_Joystick *joystick);
};

class UDPHandler {
public:
    UDPHandler();
    void sendMessage(const std::string& message);
    cv::Mat receiveFrame();

private:
    std::string base64_decode(const std::string &in);

    boost::asio::io_service io_service_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
};

class VideoHandler {
public:
    void handle_video(bool enableColorTracking);

private:
    void colorTracking();
};

#endif//SPHERO_ROBOT_CLIENT_HPP
