#ifndef MOCK_UDP_SERVER_HPP
#define MOCK_UDP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <iostream>
#include <mutex>

class MockUDPServer {
public:
    MockUDPServer(unsigned short port)
        : io_service_(), socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)), stop_flag_(false) {
        server_thread_ = std::thread([this] { this->run(); });
    }

    ~MockUDPServer() {
        stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    std::string getReceivedMessage() {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_received_message_;
    }

    void stop() {
        stop_flag_ = true;
        socket_.cancel(); // Cancel any ongoing asynchronous operations
        io_service_.stop();
    }

private:
    void run() {
        while (!stop_flag_) {
            boost::array<char, 1024> recv_buf;  // Clear the buffer on each iteration
            boost::asio::ip::udp::endpoint remote_endpoint;
            boost::system::error_code error;

            size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

            if (error) {
                if (error == boost::asio::error::operation_aborted || error == boost::asio::error::interrupted) {
                    break; // Socket was closed or operation interrupted, exit the loop
                } else {
                    std::cerr << "Error receiving message: " << error.message() << std::endl;
                    continue; // Handle or log other errors as needed
                }
            }

            if (len > 0) {
                std::lock_guard<std::mutex> lock(mutex_);
                last_received_message_ = std::string(recv_buf.data(), len); // Use len to construct the string
            }
        }
    }
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::socket socket_;
    std::thread server_thread_;
    std::string last_received_message_;
    std::mutex mutex_;
    bool stop_flag_;
};
class MockTCPServer {
public:
    MockTCPServer(unsigned short port)
        : io_service_(), acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        std::cout << "MockTCPServer constructor called" << std::endl;
        server_thread_ = std::thread([this] { this->run(); });
    }

    ~MockTCPServer() {
        std::cout << "MockTCPServer destructor called" << std::endl;
        io_service_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    bool is_accepting() const {
        return acceptor_.is_open();
    }

private:
    void run() {
        try {
            boost::asio::ip::tcp::socket socket(io_service_);
            acceptor_.accept(socket);
            std::cout << "Connection accepted" << std::endl;
        } catch (std::exception& e) {
            std::cerr << "Exception in MockTCPServer: " << e.what() << std::endl;
        }
    }

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::thread server_thread_;
};

#endif // MOCK_UDP_SERVER_HPP