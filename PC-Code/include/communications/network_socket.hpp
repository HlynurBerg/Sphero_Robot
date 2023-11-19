//
// Created by jorge on 11/19/2023.
// This code has been taken from testable_networking made by markaren and will be modified to fit our needs if necessary.

#ifndef SPHERO_ROBOT_SOCKET_HANDLER_HPP
#define SPHERO_ROBOT_SOCKET_HANDLER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace boost::asio;
using namespace boost::asio::ip;
// class to handle the socket
struct message_handler {
    // pure virtual function to handle the message
    virtual std::string handle_message(const std::string &message) = 0;
};
// class to handle the connection with the client
class connection {
public:
    // constructor that takes ownership of the socket and the message handler
    explicit connection(std::unique_ptr<tcp::socket> socket, message_handler *handler)
            : socket_(std::move(socket)), listener_(handler) {}
    // copy constructor is deleted to avoid the socket being closed twice
    connection(const connection &) = delete;
    // move constructor is deleted to avoid the socket being closed twice
    connection(const connection &&) = delete;
    // destructor to join the thread if it is still running
    ~connection() {
        if (t_.joinable()) {
            t_.join();
        }
    }

private:
    // thread to handle the message
    std::thread t_;
    // pointer to the message handler
    message_handler *listener_;
    // socket to communicate with the client
    std::unique_ptr<tcp::socket> socket_;
    // receives the size of the message
    int revSize() {
        std::array<unsigned char, 4> buf{};
        boost::asio::read(*socket_, boost::asio::buffer(buf), boost::asio::transfer_exactly(4));
        return bytes_to_int(buf);
    }
    // receives the message size and then the message
    std::string recvMsg() {
        int len = revSize();
        boost::asio::streambuf buf;
        boost::system::error_code err;
        boost::asio::read(*socket_, buf, boost::asio::transfer_exactly(len), err);
        if (err) {
            throw boost::system::system_error(err);
        }
        std::string data(boost::asio::buffer_cast<const char *>(buf.data()), len);
        return data;
    }
// sends firstly its size and then the message
void sendMsg(const std::string &msg) {
        int msgSize = static_cast<int>(msg.size());
        socket_->send(boost::asio::buffer(int_to_bytes(msgSize), 4));
        socket_->send(boost::asio::buffer(msg));
    }
// starts the message handling in a new thread
void run_handler() {
        t_ = std::thread([&] {
            try {
                while (true) {
                    std::string msg = recvMsg();
                    std::string response = listener_->handle_message(msg);
                    sendMsg(response);
                }
            } catch (const std::exception &ex) {
                std::cerr << ex.what() << std::endl;
            }
        });
    }
    // declare server as a friend class to allow access to private members
    friend class server;
};
//#TODO: add a network_helper class to handle the conversion between bytes and int
#endif//SPHERO_ROBOT_SOCKET_HANDLER_HPP
