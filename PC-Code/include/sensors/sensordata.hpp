#ifndef SPHERO_ROBOT_SENSORDATA_HPP
#define SPHERO_ROBOT_SENSORDATA_HPP
#include <SDL.h> //for User Input. Change when switching to html5 api
#include <boost/asio.hpp> //for communicatoin
#include <opencv2/opencv.hpp> //for image processing
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <thread>
#include <vector>
#include <cmath>
#include <nlohmann/json.hpp>

std::pair<float, bool>  colorTracker(cv::Mat frame);

class DataReceiver {
public:
    DataReceiver(const std::string& host, int port);

    void connect();
    void updateData();
    double getBatteryPercentage() const;
    double getDistanceMm() const;
    double getSpeedY() const;

private:
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    boost::array<char, 1024> recv_buf_;
    std::string data_buffer_;
    double battery_percentage_;
    double distance_mm_;
    double speed_y_;

    void parseData(const std::string& data);
};


#endif//SPHERO_ROBOT_SENSORDATA_HPP