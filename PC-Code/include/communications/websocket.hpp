#ifndef SPHERO_ROBOT_SERVER_HPP
#define SPHERO_ROBOT_SERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <mutex>
#include <chrono>

#include "sensors/sensor_processing.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    DataReceiver& dataReceiver_;

    net::steady_timer timer_;
    std::mutex write_mutex_; // Mutex for synchronizing write operations

public:
    explicit WebSocketSession(tcp::socket&& socket, DataReceiver& dataReceiver)
        : ws_(std::move(socket)), dataReceiver_(dataReceiver), timer_(ws_.get_executor()) {}

    void start() {
        ws_.async_accept(
                beast::bind_front_handler(&WebSocketSession::on_accept, shared_from_this()));
    }

    void start_periodic_send() {
        timer_.expires_after(std::chrono::milliseconds(10));
        timer_.async_wait(
                [self = shared_from_this()](beast::error_code ec) {
                    if (!ec) {
                        self->send_data();
                        //self->handleWebSocketMessage();
                        self->start_periodic_send(); // Reschedule
                    }
                });
    }

    void send_base64_video_frame(const std::string& base64_frame) {
        std::lock_guard<std::mutex> guard(write_mutex_); // Lock for thread safety
        std::string message = R"({"type":"video","data":")" + base64_frame + "\"}";

        //std::cout << "Sending video frame: " << message.substr(0, 30) << "... [truncated]" << std::endl;

        try {
            ws_.write(net::buffer(message));
        } catch (const std::exception& e) {
            std::cerr << "Error sending video frame: " << e.what() << std::endl;
        }
    }

private:
    void on_accept(beast::error_code ec) {
        if (ec) {
            std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
            return;
        }
        std::cout << "WebSocket connection accepted" << std::endl;

        do_read();
    }

    void do_read() {
        ws_.async_read(
                buffer_,
                beast::bind_front_handler(&WebSocketSession::on_read, shared_from_this()));
        std::cout << "Waiting to read a message..." << std::endl;
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            std::cout << "Received message: " << beast::buffers_to_string(buffer_.data()) << std::endl;

            buffer_.consume(bytes_transferred);

            // Send data periodically
            start_periodic_send();

            do_read();
        } else {
            std::cerr << "Read error: " << ec.message() << std::endl;
        }
    }

    void send_data() {
        std::lock_guard<std::mutex> guard(write_mutex_); // Lock for thread safety
        double battery_percentage = dataReceiver_.getBatteryPercentage();
        double distance_mm = dataReceiver_.getDistanceMm();
        double speed_y = dataReceiver_.getSpeedY();

        std::stringstream ss;
        ss << R"({"battery_percentage":)" << battery_percentage
           << R"(,"distance_mm":)" << distance_mm
           << R"(,"speed_y":)" << speed_y << "}";

        try {
            ws_.write(net::buffer(ss.str()));
            std::cout << "Sending data: " << ss.str() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error sending data: " << e.what() << std::endl;
        }
    }

    // Function to handle WebSocket messages
    void handleWebSocketMessage(const std::string& message) {
        if (message == "toggleColorTracking") {
            // Toggle color tracking state
            // Note: You need to decide how to access or modify the color tracking state from here
            // This might involve accessing a shared state or sending a signal to the main application logic
        }
    }

};

class WebSocketServer {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::mutex mutex_;
    DataReceiver& dataReceiver_;
    std::vector<std::shared_ptr<WebSocketSession>> sessions_; // Keep track of active sessions

public:
    WebSocketServer(net::io_context& ioc, const tcp::endpoint& endpoint, DataReceiver& dataReceiver)
        : ioc_(ioc), acceptor_(ioc, endpoint), dataReceiver_(dataReceiver) {
        do_accept();
    }

    void broadcastVideoFrame(const std::string& base64_frame) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& session : sessions_) {
            session->send_base64_video_frame(base64_frame);
        }

        //std::cout << "Broadcasted video frame to " << sessions_.size() << " clients." << std::endl;
    }

private:
    void do_accept() {
        acceptor_.async_accept(
                net::make_strand(ioc_),
                [this](beast::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        // Create a new session and add it to the sessions vector
                        auto session = std::make_shared<WebSocketSession>(std::move(socket), dataReceiver_);
                        std::lock_guard<std::mutex> lock(mutex_);
                        sessions_.push_back(session);
                        session->start();
                    }

                    do_accept();
                });
    }
};

#endif//SPHERO_ROBOT_SERVER_HPP
