#include "catch2/catch_test_macros.hpp"
#include "communications/client.hpp"
#include "communications/mock_server.hpp"
#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>

// currently not receiving in UDPHandler #TODO: Faultfinding on UDPHandler Base64 Receive Test
TEST_CASE("UDPHandler Base64 Receive Test", "[UDPHandler]") {
    std::cout << "Starting MockUDPServer on port 6001" << std::endl;
    MockUDPServer mockServer(6001);
    std::cout << "Connected on port 6001" << std::endl;

    UDPHandler udpHandler("127.0.0.1", 6001);

    SECTION("Sending Base64 Encoded 'Hello World'") {
        std::cout << "Preparing to send base64 encoded message" << std::endl;
        std::string base64Message = "SGVsbG8gV29ybGQ=";  // Base64 for 'Hello World'
        mockServer.sendBase64Message(base64Message, "127.0.0.1", 6001);

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give time for the message to be received
        std::cout << "Attempting to receive message in UDPHandler" << std::endl;

        std::string receivedMessage = udpHandler.ReceiveBase64Frame();
        std::cout << "Received message: " << receivedMessage << std::endl;

        REQUIRE(udpHandler.Base64Decode(receivedMessage) == "Hello World");
    }
}

TEST_CASE("Base64 Decoding Test", "[base64]") {
    std::cout << "Running test" << std::endl;

    // Create an instance of UDPHandler
    UDPHandler udpHandler("127.0.0.1", 6001);

    std::string encoded = "SGVsbG8sIHdvcmxkIQ=="; // "Hello, world!" in base64
    std::string expectedDecoded = "Hello, world!";

    std::string actualDecoded = udpHandler.Base64Decode(encoded);

    std::cout << "Decoded string: " << actualDecoded << std::endl;

    REQUIRE(actualDecoded == expectedDecoded);
}

// Function to save a mock image
void saveMockImage() {
    cv::Mat mockImage(100, 100, CV_8UC3, cv::Scalar(0, 0, 255)); // A simple red image
    if (cv::imwrite("mock_image.png", mockImage)) {
        std::cout << "Mock image successfully saved to 'mock_image.png'" << std::endl;
    } else {
        std::cerr << "Failed to save mock image to 'mock_image.png'" << std::endl;
    }
}

TEST_CASE("cv::imdecode PNG Test", "[imdecode]") {
    // Save the mock image
    saveMockImage();

    // Read the file into a buffer
    std::ifstream file("mock_image.png", std::ios::binary);
    if (file.good()) {
        std::cout << "PNG file 'mock_image.png' opened successfully." << std::endl;
    } else {
        std::cerr << "Failed to open 'mock_image.png'" << std::endl;
    }
    REQUIRE(file.good());

    std::vector<uchar> buffer(std::istreambuf_iterator<char>(file), {});
    std::cout << "Read " << buffer.size() << " bytes from the file." << std::endl;

    REQUIRE_FALSE(buffer.empty());

    // Decode the buffer to a cv::Mat object
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    if (!image.empty()) {
        std::cout << "Image successfully decoded. Dimensions: " << image.cols << " x " << image.rows << std::endl;
    } else {
        std::cerr << "Failed to decode the image." << std::endl;
    }

    REQUIRE_FALSE(image.empty());
}

TEST_CASE("TCPHandler Connection Test", "[TCPHandler]") {
    unsigned short test_port = 6003; // Choose an appropriate port number
    MockTCPServer mockServer(test_port);

    std::string host = "127.0.0.1"; // Localhost
    TCPHandler tcpHandler(host, test_port);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    REQUIRE(tcpHandler.isConnected());

}

TEST_CASE("UDPHandler Connection Test", "[UDPHandler]") {
    unsigned short test_port = 6001; //port number
    MockUDPServer mockServer(test_port);

    std::string host = "127.0.0.1"; // Localhost
    UDPHandler udpHandler(host, test_port);

    // Send a test message to see if the server receives it
    std::string test_message = "Hello";
    udpHandler.Handshake(test_message);

}

//#TODO for the TCPHandler add data sending, receiving and error handling tests
//#TODO add a handshake test and a error handling test for the UDPHandler


