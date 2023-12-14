#include <communications/client.hpp>
#include <control/motorcontroller.hpp>


void handle_controlling(TankSteering& steer, std::mutex& steer_mutex) {
    // Your TCP client logic here
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6000);
    boost::asio::ip::tcp::socket socket(io_service);
    // Now you can use 'steer' within this context

    try {
        socket.connect(endpoint);
    } catch (boost::system::system_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    TankSteering localCopy;
    while (true) { // Loop to continuously send data

        { // Scoped lock
            std::lock_guard<std::mutex> lock(steer_mutex);
            localCopy = steer; // Copying the shared data under the lock
        }
        std::string command = std::to_string(localCopy.leftBelt) + ", " + std::to_string(localCopy.rightBelt) + "\n";
        // send localCopy to server
        boost::system::error_code error;
        socket.write_some(boost::asio::buffer(command), error);


        if (error) {
            std::cerr << "Error while sending data: " << error.message() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Control the update rate
    }

    //se på referanse
    //TankSteering result = referanse
    std::string command = std::to_string(steer.leftBelt) + ", " + std::to_string(steer.rightBelt) + "\n";
    boost::system::error_code error;
    socket.write_some(boost::asio::buffer(command), error);

    std::cout << "Sending Command: " << command << std::endl;

    if (error) {
        std::cerr << "Error while sending data: " << error.message() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}


UDPHandler::UDPHandler(const std::string& host, int port)
    : io_service_(), socket_(io_service_), remote_endpoint_(boost::asio::ip::address::from_string(host), port) {
    socket_.open(boost::asio::ip::udp::v4());
    sendMessage("Hello"); // Send initial message upon creation
}

void UDPHandler::sendMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
}

// For HTML/WS streaming (returns base64 encoded string)
std::string UDPHandler::receiveBase64Frame() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    boost::array<char, 65536> recv_buf;
    size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint_);
    return std::string(recv_buf.data(), len);
}


std::string UDPHandler::base64_decode(const std::string &in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (uchar c: in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
