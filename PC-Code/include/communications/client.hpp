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
#include <control/motorcontroller.hpp>
#include <mutex>
// get a reference to the TankSteering struct and pass it to the function


void handle_controlling(TankSteering& steer, std::mutex& steer_mutex);


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

void handle_video(cv::Mat& frame, std::mutex& frame_mutex);

#endif//SPHERO_ROBOT_CLIENT_HPP