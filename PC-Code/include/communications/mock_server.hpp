#ifndef MOCK_SERVER_HPP
#define MOCK_SERVER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <functional>

class MockServer {
public:
    MockServer(short tcp_port, short udp_port);
    void run();

private:
    void start_tcp_accept();
    void handle_tcp_connection(boost::asio::ip::tcp::socket socket);
    void start_udp_receive();
    void handle_udp_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_tcp_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor tcp_acceptor_;
    boost::asio::ip::udp::socket udp_socket_;
    boost::asio::ip::udp::endpoint udp_remote_endpoint_;
    std::array<char, 1024> udp_recv_buffer_;
};

#endif // MOCK_SERVER_HPP
