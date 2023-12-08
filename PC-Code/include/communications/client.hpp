#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <boost/asio.hpp>
#include <iostream>

class TCPServer {
public:
    TCPServer(short port);
    void run();

private:
    void acceptConnections();

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // TCP_SERVER_HPP

