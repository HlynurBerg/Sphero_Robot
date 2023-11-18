//
// Created by jorge on 11/9/2023.
//

#ifndef SPHERO_ROBOT_SENSORDATA_HPP
#define SPHERO_ROBOT_SENSORDATA_HPP

//TODO: All the below works in the locally test code, but not in the actual code. Fix it <3
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json;

class DataReceiver {
public:
    DataReceiver(const std::string& host, int port)
        : io_service_(), socket_(io_service_), endpoint_(tcp::endpoint(boost::asio::ip::address::from_string(host), port)) {
        connect();
    }

    void connect() {
        socket_.connect(endpoint_);
    }

    void updateData() {
        boost::system::error_code error;
        while(socket_.available()) {
            size_t len = socket_.read_some(boost::asio::buffer(recv_buf_), error);
            if (error && error != boost::asio::error::eof) {
                std::cerr << "Receive failed: " << error.message() << std::endl;
                return;
            }
            data_buffer_ += std::string(recv_buf_.data(), len);
        }

        // Process complete JSON messages
        while(true) {
            size_t pos = data_buffer_.find('\n');
            if (pos != std::string::npos) {
                std::string json_message = data_buffer_.substr(0, pos);
                data_buffer_.erase(0, pos + 1);
                parseData(json_message);
            } else {
                break;
            }
        }
    }

    double getBatteryPercentage() const { return battery_percentage_; }
    double getDistanceMm() const { return distance_mm_; }
    double getSpeedY() const { return speed_y_; }

private:
    boost::asio::io_service io_service_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    boost::array<char, 1024> recv_buf_;
    std::string data_buffer_;
    double battery_percentage_;
    double distance_mm_;
    double speed_y_;

    void parseData(const std::string& data) {
        try {
            auto j = json::parse(data);
            battery_percentage_ = j["Battery"]["percentage"];
            distance_mm_ = j["Distance"];
            speed_y_ = j["Speed"]["Velocity"]["Y"];
        } catch (const std::exception& e) {
            std::cerr << "JSON Parsing error: " << e.what() << std::endl;
            std::cerr << "Received data: " << data << std::endl; // For debugging
        }
    }
};

#endif//SPHERO_ROBOT_SENSORDATA_HPP
