#include <communications/client.hpp>
#include <control/motorcontroller.hpp>

// Definitions of member functions of TCPHandler
TCPHandler::TCPHandler(const std::string& host, int port)
    : io_service_(), socket_(io_service_), endpoint_(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port)) {
    Connect();
}

void TCPHandler::HandleControlling(TankSteering &steer, std::mutex &steer_mutex) {
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

    TankSteering local_copy;
    while (true) { // Loop to continuously send data
        //TODO: Threadsafe queue
        { // Scoped lock
            std::lock_guard<std::mutex> lock(steer_mutex);
            local_copy = steer; // Copying the shared data under the lock
        }
        std::string command = std::to_string(local_copy.left_belt_) + ", " + std::to_string(local_copy.right_belt_) + "\n";
        // send local_copy to server
        boost::system::error_code error;
        socket.write_some(boost::asio::buffer(command), error);


        if (error) {
            std::cerr << "Error while sending data: " << error.message() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Control the update rate
    }
}

void TCPHandler::Connect() {
    socket_.connect(endpoint_);
}

void TCPHandler::UpdateData() {
    boost::system::error_code error;
    while(socket_.available()) {
        size_t len = socket_.read_some(boost::asio::buffer(recv_buf_), error);
        if (error && error != boost::asio::error::eof) {
            std::cerr << "Receive failed: " << error.message() << std::endl;
            return;
        }
        data_buffer_ += std::string(recv_buf_.data(), len);
    }

    // Process complete JSON messages
    while(true) {
        size_t pos = data_buffer_.find('\n');
        if (pos != std::string::npos) {
            std::string json_message = data_buffer_.substr(0, pos);
            data_buffer_.erase(0, pos + 1);
            ParseData(json_message);
        } else {
            break;
        }
    }
}

double TCPHandler::GetBatteryPercentage() const {
    return battery_percentage_;
}

double TCPHandler::GetDistanceMm() const {
    return distance_mm_;
}

double TCPHandler::GetSpeedY() const {
    return speed_y_;
}

void TCPHandler::ParseData(const std::string& data) {
    try {
        auto j = nlohmann::json::parse(data);
        battery_percentage_ = j["Battery"]["percentage"];
        distance_mm_ = j["Distance"];
        speed_y_ = j["Speed"]["Velocity"]["Y"];
    } catch (const std::exception& e) {
        std::cerr << "JSON Parsing error: " << e.what() << std::endl;
        std::cerr << "Received data: " << data << std::endl; // For debugging
    }
}

UDPHandler::UDPHandler(const std::string& host, int port)
    : io_service_(), socket_(io_service_), remote_endpoint_(boost::asio::ip::address::from_string(host), port) {
    socket_.open(boost::asio::ip::udp::v4());
    Handshake("Hello"); // Send initial message upon creation
}

void UDPHandler::Handshake(const std::string& message) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
}

std::string UDPHandler::ReceiveBase64Frame() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    boost::array<char, 65536> receive_buffer{};
    size_t receive_length = socket_.receive_from(boost::asio::buffer(receive_buffer), remote_endpoint_);
    return {receive_buffer.data(), receive_length};
}

//TODO: if time: use boost base64 decoding instead of this custom implementation
std::string UDPHandler::Base64Decode(const std::string& encoded_string) {
    std::string decoded_string;
    std::vector<int> base64_index_map(256, -1);
    const std::string kBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Initialize the index map for Base64 characters
    for (int i = 0; i < 64; ++i) {
        base64_index_map[kBase64Chars[i]] = i;
    }

    int accumulator = 0;  // Accumulates the bit stream
    int bit_count = -8;   // Counts the number of bits in 'accumulator'
    for (unsigned char current_char : encoded_string) {
        if (base64_index_map[current_char] == -1) break;  // Stop decoding on invalid character

        // Accumulate bits from the Base64 character
        accumulator = (accumulator << 6) + base64_index_map[current_char];
        bit_count += 6;

        // If there are enough bits, extract a byte
        if (bit_count >= 0) {
            decoded_string.push_back(static_cast<char>((accumulator >> bit_count) & 0xFF));
            bit_count -= 8;
        }
    }
    return decoded_string;
}
