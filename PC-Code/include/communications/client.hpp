#ifndef SPHERO_ROBOT_CLIENT_HPP
#define SPHERO_ROBOT_CLIENT_HPP

//#include <SDL.h> //for User Input. Change when switching to html5 api
#include <boost/asio.hpp> //for communication
#include <opencv2/opencv.hpp> //for image processing
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <thread>
//#include <vector>
//#include <cmath>
#include <control/motorcontroller.hpp>
#include <mutex>
//#include <communications/thread_safe_queue.hpp>
// get a reference to the TankSteering struct and pass it to the function


void handle_controlling(TankSteering& steer, std::mutex& steer_mutex);


class UDPHandler {
public:
    UDPHandler(const std::string& host, int port);
    void sendMessage(const std::string& message);

    std::string receiveBase64Frame();
    std::string base64_decode(const std::string &in);

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::socket socket_;
    std::mutex socket_mutex_;  // Mutex to protect socket access

    boost::asio::ip::udp::endpoint remote_endpoint_;
};

#endif//SPHERO_ROBOT_CLIENT_HPP