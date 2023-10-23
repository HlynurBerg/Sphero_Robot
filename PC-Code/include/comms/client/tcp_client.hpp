//
// Created by jorge on 10/16/2023.
//
#ifndef TESTABLE_NETWORKING_TCP_CLIENT_HPP
#define TESTABLE_NETWORKING_TCP_CLIENT_HPP

#include <boost/asio.hpp>

#include <array>
#include <iostream>
#include <string>

#include "comms/communicationhandler.hpp"


using boost::asio::ip::tcp;

class tcp_client {

public:
    tcp_client(const std::string &host, int port) {

        auto endpoints = resolver_.resolve(host, std::to_string(port));
        boost::asio::connect(socket_, endpoints);
    }

    void send(const std::string& msg) {
        int msgSize = static_cast<int>(msg.size());

        socket_.send(boost::asio::buffer(int_to_bytes(msgSize), 4));
        socket_.send(boost::asio::buffer(msg));
    }

    std::string recv() {
        boost::system::error_code error;

        std::array<unsigned char, 4> sizeBuf{};
        boost::asio::read(socket_, boost::asio::buffer(sizeBuf), boost::asio::transfer_exactly(4), error);
        if (error) {
            throw boost::system::system_error(error);
        }

        boost::asio::streambuf buf;
        size_t len = boost::asio::read(socket_, buf, boost::asio::transfer_exactly(bytes_to_int(sizeBuf)), error);
        if (error) {
            throw boost::system::system_error(error);
        }

        std::string data(boost::asio::buffer_cast<const char *>(buf.data()), len);

        return data;
    }

private:
    boost::asio::io_service io_service_;
    tcp::resolver resolver_{io_service_};
    tcp::socket socket_{io_service_};
};

#endif//TESTABLE_NETWORKING_TCP_CLIENT_HPP
