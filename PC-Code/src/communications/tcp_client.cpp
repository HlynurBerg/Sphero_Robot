#include <communications/client.hpp>
#include <control/motorcontroller.hpp>

void NetworkHandler::handle_controlling(SDL_Joystick *joystick) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("10.25.46.49"), 6000);
    boost::asio::ip::tcp::socket socket(io_service);

    try {
        socket.connect(endpoint);
    } catch (boost::system::system_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) { // Process SDL events
            if (e.type == SDL_QUIT) {
                // Handle quit event if necessary
                return;
            }
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
        TankSteering result = getTankSteering(keyboardState, joystick);

        std::string command = std::to_string(result.leftBelt) + ", " + std::to_string(result.rightBelt) + "\n";
        boost::system::error_code error;
        socket.write_some(boost::asio::buffer(command), error);

        std::cout << "Sending Command: " << command << std::endl;

        if (error) {
            std::cerr << "Error while sending data: " << error.message() << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

UDPHandler::UDPHandler()
    : io_service_(), socket_(io_service_), remote_endpoint_(boost::asio::ip::address::from_string("10.25.46.49"), 6001) {
    socket_.open(boost::asio::ip::udp::v4());//TODO Se på dette Robert! Ikke helt sikker her
    sendMessage("Hello"); // Send initial message upon creation
}

void UDPHandler::sendMessage(const std::string& message) {
    socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
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

void VideoHandler::handle_video(bool enableColorTracking) {
    if (enableColorTracking) {
        colorTracking();
    } else {
        try {
            std::cout << "Starting video thread" << std::endl;
            UDPHandler udpHandler; // Using the new UDPHandler class

            while (true) {
                std::cout << "Starting video loop" << std::endl;
                cv::Mat frame = udpHandler.receiveFrame(); // Receiving frame using UDPHandler

                if (!frame.empty()) {
                    std::cout << "Received frame of size: " << frame.size() << std::endl;
                    cv::imshow("Received Video", frame);
                } else {
                    std::cerr << "Received empty frame" << std::endl;
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
}

void VideoHandler::colorTracking() {
    try {
        UDPHandler udpHandler;

        cv::Scalar lower_bound(13, 175, 50);
        cv::Scalar upper_bound(23, 255, 255);
        cv::Mat hsv, mask, segmented;
        std::vector<std::vector<cv::Point>> contours;
        int min_contour_area = 500;

        while (true) {
            std::cout << "Starting color tracking" << std::endl;
            cv::Mat image = udpHandler.receiveFrame();
            if (image.empty()) {
                std::cerr << "Could not decode frame!" << std::endl;
                continue;
            }

            // Color detection and contour finding
            cvtColor(image, hsv, cv::COLOR_BGR2HSV);
            inRange(hsv, lower_bound, upper_bound, mask);
            bitwise_and(image, image, segmented, mask);
            findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            double weightedSumX = 0.0, weightedSumY = 0.0, totalArea = 0.0;

            for (const auto &contour: contours) {
                double area = contourArea(contour);
                if (area > min_contour_area) {
                    cv::Moments M = moments(contour);
                    if (M.m00 != 0) {
                        cv::Point2f centroid(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));
                        weightedSumX += centroid.x * area;
                        weightedSumY += centroid.y * area;
                        totalArea += area;

                        circle(image, centroid, 5, cv::Scalar(255, 0, 0), -1);
                        std::string label = "Size: " + std::to_string(area) + " px";
                        putText(image, label, centroid, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1,
                                cv::LINE_AA);
                    }
                }
            }

            if (totalArea > 0) {
                cv::Point2f overallCentroid(static_cast<float>(weightedSumX / totalArea),
                                            static_cast<float>(weightedSumY / totalArea));
                circle(image, overallCentroid, 10, cv::Scalar(0, 0, 255), -1);

                // Calculate the horizontal difference from the center of the screen
                float screenCenterX = segmented.cols / 2.0f;
                float difference = overallCentroid.x - screenCenterX;

                std::cout << "Difference: " << difference << std::endl;

                //TODO: Implement an autonome steering function to keep the ball in the center of the screen

            }

            imshow("Color Tracking", image); //"image" for raw and "segmented" for only the color

            if (cv::waitKey(10) == 'q') {
                break;
            }
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

