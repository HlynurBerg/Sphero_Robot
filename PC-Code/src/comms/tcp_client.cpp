// include the clients.hpp file from the client folder in the comms library
#include "comms/client/clients.hpp"


int main(int argc, char **argv) {

    std::string host = "127.0.0.1";
    int port = 9090;
    if (argc == 3) {
        // assuming <hostname> <port>
        host = argv[1];
        port = std::stoi(argv[2]);
    }

    try {
        clients client(host, port);

        for (int i = 0; i < 10; i++) {
            client.send("Per_" + std::to_string(i));
            auto result = client.recv();
            std::cout << "Got: " << result;
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}