#ifndef SPHERO_ROBOT_CLIENT_HPP
#define SPHERO_ROBOT_CLIENT_HPP

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <thread>
#include <control/motorcontroller.hpp>
#include <mutex>


void HandleControlling(TankSteering& steer, std::mutex& steer_mutex);


class UDPHandler {
public:
    UDPHandler(const std::string& host, int port);
    void Handshake(const std::string& message);

    std::string ReceiveBase64Frame();
    std::string Base64Decode(const std::string &encoded_string);

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::socket socket_;
    std::mutex socket_mutex_;  // Mutex to protect socket access

    boost::asio::ip::udp::endpoint remote_endpoint_;
};

#endif//SPHERO_ROBOT_CLIENT_HPP