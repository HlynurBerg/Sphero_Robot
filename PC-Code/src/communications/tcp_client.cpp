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


UDPHandler::UDPHandler()
    : io_service_(), socket_(io_service_), remote_endpoint_(boost::asio::ip::address::from_string("10.25.46.49"), 6001) {
    socket_.open(boost::asio::ip::udp::v4());
    sendMessage("Hello"); // Send initial message upon creation
}

void UDPHandler::sendMessage(const std::string& message) {
    socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
}

// For HTML/WS streaming (returns base64 encoded string)
std::string UDPHandler::receiveBase64Frame() {
    boost::array<char, 65536> recv_buf;
    size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint_);
    return std::string(recv_buf.data(), len);
}

cv::Mat UDPHandler::receiveFrame() {
    boost::array<char, 65536> recv_buf;
    size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), remote_endpoint_);
    std::string encoded_data(recv_buf.begin(), recv_buf.begin() + len);
    std::string decoded_data = base64_decode(encoded_data);
    std::vector<uchar> buf(decoded_data.begin(), decoded_data.end());
    return cv::imdecode(buf, cv::IMREAD_COLOR);
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

void handle_video(cv::Mat& frame, std::mutex& frame_mutex){
    try {
        UDPHandler udpHandler; // Using the new UDPHandler class

        while (true) {
            cv::Mat local_frame = udpHandler.receiveFrame(); // Receiving local_frame using UDPHandler
            if (!local_frame.empty()) {

                { // Scoped lock
                    std::lock_guard<std::mutex> lock(frame_mutex);
                    frame = local_frame; // Copying the shared data under the lock
                }
                //cv::imshow("Video", local_frame);
            } else {
                std::cerr << "Received empty local_frame" << std::endl;
            }

            if (cv::waitKey(10) == 'q') {
                break;
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

// New functions from Robert ? <- Waz is daz?
//
std::vector<std::string> splitter(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Connects to the server
int connect_to_server(const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket" << std::endl;
        return -1;
    }

    sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed. Error");
        return -1;
    }

    std::cout << "Connected to server." << std::endl;
    return sock;
}