#ifndef SPHERO_ROBOT_SERVER_HPP
#define SPHERO_ROBOT_SERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include "sensors/sensor_processing.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    TCPHandler &data_receiver_;

    net::steady_timer timer_;
    std::mutex write_mutex_; // Mutex for synchronizing write operations
    std::atomic<bool>&color_tracking_state_;
    std::atomic<float>&new_max_speed_;

public:
    WebSocketSession(tcp::socket&& socket, TCPHandler &data_receiver, std::atomic<bool>&color_tracking_state, std::atomic<float>&new_max_speed)
        : ws_(std::move(socket)), data_receiver_(data_receiver), timer_(ws_.get_executor()), color_tracking_state_(color_tracking_state), new_max_speed_(new_max_speed) {}

    void Start() {
        ws_.async_accept(
                beast::bind_front_handler(&WebSocketSession::OnAccept, shared_from_this()));
    }

    void StartPeriodicSend() {
        timer_.expires_after(std::chrono::milliseconds(10));
        timer_.async_wait(
                [self = shared_from_this()](beast::error_code ec) {
                    if (!ec) {
                        self->SendData();
                        self->StartPeriodicSend(); // Reschedule
                    }
                });
    }

    void SendBase64VideoFrame(const std::string& base64_frame) {
        std::lock_guard<std::mutex> guard(write_mutex_); // Lock for thread safety
        std::string message = R"({"type":"video","data":")" + base64_frame + "\"}";

        try {
            ws_.write(net::buffer(message));
        } catch (const std::exception& e) {
            std::cerr << "Error sending video frame: " << e.what() << std::endl;
        }
    }

private:
    void OnAccept(beast::error_code ec) {
        if (ec) {
            std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
            return;
        }
        std::cout << "WebSocket connection accepted" << std::endl;

        DoRead();
    }

    void DoRead() {
        ws_.async_read(
                buffer_,
                beast::bind_front_handler(&WebSocketSession::OnRead, shared_from_this()));
        std::cout << "Waiting to read a message..." << std::endl;
    }

    void OnRead(beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            std::string received_msg = beast::buffers_to_string(buffer_.data());
            std::cout << "Received message: " << beast::buffers_to_string(buffer_.data()) << std::endl;

            buffer_.consume(bytes_transferred);

            // Handle the WebSocket message
            HandleWebSocketMessage(received_msg);

            // Send data periodically
            StartPeriodicSend();

            DoRead();
        } else {
            std::cerr << "Read error: " << ec.message() << std::endl;
        }
    }

    void SendData() {
        std::lock_guard<std::mutex> guard(write_mutex_); // Lock for thread safety
        double battery_percentage = data_receiver_.GetBatteryPercentage();
        double distance_mm = data_receiver_.GetDistanceMm();
        double speed_y = data_receiver_.GetSpeedY();

        std::stringstream ss;
        ss << R"({"battery_percentage":)" << battery_percentage
           << R"(,"distance_mm":)" << distance_mm
           << R"(,"speed_y":)" << speed_y << "}";

        try {
            ws_.write(net::buffer(ss.str()));
        } catch (const std::exception& e) {
            std::cerr << "Error sending data: " << e.what() << std::endl;
        }
    }

    void HandleWebSocketMessage(const std::string& message) {
        try {
            auto j = nlohmann::json::parse(message);

            // Toggle color tracking
            if (j["type"] == "toggleColorTracking") {
                color_tracking_state_ = !color_tracking_state_.load();
                std::cout << "Color tracking toggled to " << (color_tracking_state_ ? "enabled" : "disabled") << std::endl;
            }

            // Handle setMaxSpeed message
            if (j["type"] == "setMaxSpeed") {
                std::string speed_string = j["value"].get<std::string>();
                float speed = std::stof(speed_string);
                new_max_speed_.store(speed);
                std::cout << "Max speed set to " << new_max_speed_.load() << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        }
    }

};

class WebSocketServer {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::mutex mutex_;
    TCPHandler &data_receiver_;
    std::vector<std::shared_ptr<WebSocketSession>> sessions_; // Keep track of active sessions
    std::atomic<bool>&color_tracking_state_;
    std::atomic<float>&new_max_speed_;

public:
    WebSocketServer(net::io_context &ioc, const tcp::endpoint &endpoint, TCPHandler &dataReceiver, std::atomic<bool> &colorTrackingState, std::atomic<float> &maxSpeed)
        : ioc_(ioc), acceptor_(ioc, endpoint), data_receiver_(dataReceiver), color_tracking_state_(colorTrackingState), new_max_speed_(maxSpeed) {
        DoAccept();
    }

    void BroadcastVideoFrame(const std::string& base64_frame) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& session : sessions_) {
            session->SendBase64VideoFrame(base64_frame);
        }
    }

private:
    void DoAccept() {
        acceptor_.async_accept(
                net::make_strand(ioc_),
                [this](beast::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        // Create a new session and add it to the sessions vector
                        auto session = std::make_shared<WebSocketSession>(std::move(socket), data_receiver_, color_tracking_state_, new_max_speed_);
                        std::lock_guard<std::mutex> lock(mutex_);
                        sessions_.push_back(session);
                        session->Start();
                    }

                    DoAccept();
                });
    }
};

#endif//SPHERO_ROBOT_SERVER_HPP
