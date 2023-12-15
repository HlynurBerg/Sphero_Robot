#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"
#include "communications/client.hpp"
#include "communications/mock_server.hpp"
#include "sensors/sensor_processing.hpp"
#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>



//the definitions of TankSteering and handle_controlling() are accessible
extern void handle_controlling(TankSteering& steer, std::mutex& steer_mutex);
extern void handle_video(cv::Mat& frame, std::mutex& frame_mutex);

// Mock data for testing
const std::string mockTCPData = "Mock TCP Command";
const cv::Mat mockImage(100, 100, CV_8UC3, cv::Scalar(0, 0, 255));  // A simple red image for testing




TEST_CASE("UDPHandler Base64 Receive Test", "[UDPHandler]") {
    std::cout << "Starting MockUDPServer on port 6001" << std::endl;
    MockUDPServer mockServer(6001);
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
/*
void test_tcp_client() {
    std::cout << "Client is attempting to connect to the server." << std::endl;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 6000);
    std::cout << "Client has attempted to connect to the server." << std::endl;

    try {
        socket.connect(endpoint);
        std::cout << "Connected to server for test." << std::endl;

        // Example data
        int intValue = 42;
        float floatValue = 3.14f;
        std::string message = std::to_string(intValue) + ", " + std::to_string(floatValue);

        // Send data
        std::cout << "Sending data: " << message << std::endl;
        boost::asio::write(socket, boost::asio::buffer(message));

        // Buffer for receiving responses
        char reply[1024];

        // Receive server response
        size_t reply_length = socket.read_some(boost::asio::buffer(reply, 1024));
        std::cout << "Server reply: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";

        // Receive sensor data from server
        std::fill(std::begin(reply), std::end(reply), 0);  // Clear the buffer before the next read
        size_t sensor_data_length = socket.read_some(boost::asio::buffer(reply, 1024));
        std::cout << "Client received sensor data: ";
        std::cout.write(reply, sensor_data_length);
        std::cout << "\n";
    } catch (std::exception& e) {
        std::cerr << "Exception in test_tcp_client: " << e.what() << "\n";
    }
}



TEST_CASE("TCP Communication Test") {
    std::cout << "TCP Communication Test started" << std::endl;

    MockServer server(6000, 6001);
    std::thread testThread(test_tcp_client);

    testThread.join();
}
*/


TEST_CASE("Base64 Decoding Test", "[base64]") {
    std::cout << "Running test" << std::endl;

    // Create an instance of UDPHandler
    // You need to provide the host and port to the constructor
    UDPHandler udpHandler("127.0.0.1", 6001);

    std::string encoded = "SGVsbG8sIHdvcmxkIQ=="; // "Hello, world!" in base64
    std::string expectedDecoded = "Hello, world!";

    // Now use the udpHandler instance to call base64_decode
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
    // Save the mock image as a PNG file
    saveMockImage();

    // Read the PNG file into a buffer
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
/*
TEST_CASE("UDPHandler Test", "[UDPHandler]") {
    // Start the mock server
    short udp_port = 12345; // Example port
    MockServer mockServer(0, udp_port); // 0 for TCP port as it's not used here

    // Initialize UDPHandler to send data to the mock server
    UDPHandler udpHandler("127.0.0.1", udp_port);

    // Create a test image or any data that UDPHandler is supposed to send
    cv::Mat testImage = cv::Mat::eye(10, 10, CV_8UC3); // Example image

    // Send data through UDPHandler
    // ...

    // Verify the data is received by the mock server
    // This might involve additional implementation in the mock server to track received data
}
*/
/*
TEST_CASE("DataReceiver sends data correctly", "[DataReceiver]") {
    MockTCPServer mockTcpServer(6000);
    mockTcpServer.start();

    DataReceiver dataReceiver("127.0.0.1", 6000); // Assuming localhost and the port

    SECTION("Sending a test message") {
        dataReceiver.sendData("Test message");
        std::string receivedMessage = mockTcpServer.getLastReceivedMessage();
        REQUIRE(receivedMessage == "Test message");
    }

    mockTcpServer.stop();
}
*/


/*
TEST_CASE("Handshake UDP Test") {
    // Create an instance of UDPHandler
    boost::asio::io_service io_service;
    MockServer mockServer(io_service, 6001);


    // Define the expected initial message
    std::string expectedMessage = "Hello";

    // Set the next message to be received by MockServer
    mockServer.SetNextMessage(expectedMessage);

    // Perform the handshake
    udpHandler.Handshake(expectedMessage);

    // Simulate receiving the message using MockServer
    std::string receivedMessage = mockServer.ReceiveMessage(); // Simulate receiving a message

    // Add print statements to check the values
    INFO("Expected Message: " << expectedMessage);
    INFO("Received Message: " << receivedMessage);

    // Check if the received message matches the expected message
    REQUIRE(receivedMessage == expectedMessage);
}
*/



/*
TEST_CASE("Handshake Test") {
    // Create a MockServer instance
    MockServer mockServer("127.0.0.1", 6001);

    // Define the message to send (Base64 encoded "Hello World")
    std::string message = "SGVsbG8gV29ybGQ=";

    // Send the message to UDPHandler using MockServer
    mockServer.SendMessage(message);

    // Now, perform the handshake with UDPHandler
    UDPHandler udpHandler("127.0.0.1", 6001);
    std::string receivedMessage = udpHandler.ReceiveBase64Frame();

    // Decode the received message
    std::string decodedMessage = udpHandler.Base64Decode(receivedMessage);

    // Add print statements to check the values
    INFO("Expected Message: " << "Hello World");
    INFO("Received Message: " << decodedMessage);

    // Check if the received message matches the expected message
    REQUIRE(decodedMessage == "Hello World");
}

*/

