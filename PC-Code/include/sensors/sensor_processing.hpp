#ifndef SPHERO_ROBOT_SENSOR_PROCESSING_HPP
#define SPHERO_ROBOT_SENSOR_PROCESSING_HPP

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <vector>
#include <cmath>
#include <nlohmann/json.hpp>

std::pair<float, bool> ColorTracker(cv::Mat image);

class DataReceiver {
public:
    DataReceiver(const std::string& host, int port);

    void connect();
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


#endif//SPHERO_ROBOT_SENSOR_PROCESSING_HPP