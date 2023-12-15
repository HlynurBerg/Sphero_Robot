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
        : io_service_(), socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)) {
        std::cout << "MockUDPServer constructor called" << std::endl;
        server_thread_ = std::thread([this] { this->run(); });
    }

    ~MockUDPServer() {
        std::cout << "MockUDPServer destructor called" << std::endl;
        io_service_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    void sendBase64Message(const std::string& message, const std::string& host, unsigned short port) {
        try {
            std::cout << "Sending base64 message to " << host << "127.0.0.1" << port << std::endl;
            boost::asio::ip::udp::endpoint remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(host), port);
            socket_.send_to(boost::asio::buffer(message), remote_endpoint);
            std::cout << "Message sent: " << message << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
        }
    }

private:
    void run() {
        io_service_.run();
    }

    boost::asio::io_service io_service_;
    boost::asio::ip::udp::socket socket_;
    std::thread server_thread_;
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
