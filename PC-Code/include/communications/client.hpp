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
#include <nlohmann/json.hpp>


class TCPHandler {
public:
    TCPHandler(const std::string& host, int port);

    static void HandleControlling(TankSteering& steer, std::mutex& steer_mutex);
    void Connect();
    void UpdateData();
    double GetBatteryPercentage() const;
    double GetDistanceMm() const;
    double GetSpeedY() const;

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    boost::array<char, 1024> recv_buf_;
    std::string data_buffer_;
    double battery_percentage_;
    double distance_mm_;
    double speed_y_;

    void ParseData(const std::string& data);
};

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