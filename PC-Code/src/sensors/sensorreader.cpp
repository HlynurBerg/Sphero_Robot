#include <sensors/sensordata.hpp>

std::pair<float, bool> colorTracker(cv::Mat image) {
    try {
        //TODO: Define color bounds as a variable, let used decide
        cv::Scalar lower_bound(10, 150, 50);
        cv::Scalar upper_bound(25, 255, 255);
        // Define smallest detected object
        int min_contour_area = 250;
        bool isValid = false;
        float difference = 0.0f;
        cv::Mat hsv, mask, segmented;
        std::vector<std::vector<cv::Point>> contours;

        if (image.empty()) {
            std::cerr << "Could not decode frame!" << std::endl;
        }

        // Color and contour detection
        cvtColor(image, hsv, cv::COLOR_BGR2HSV);
        inRange(hsv, lower_bound, upper_bound, mask);
        bitwise_and(image, image, segmented, mask);
        findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        double weightedSumX = 0.0, weightedSumY = 0.0, totalArea = 0.0;

        for (const auto &contour: contours) {
            double area = contourArea(contour);
            if (area > min_contour_area) {
                isValid = true; // Valid object found
                cv::Moments M = moments(contour);
                if (M.m00 != 0) {
                    cv::Point2f centroid(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));
                    weightedSumX += centroid.x * area;
                    weightedSumY += centroid.y * area;
                    totalArea += area;

                    /*
                    circle(image, centroid, 5, cv::Scalar(255, 0, 0), -1);
                    std::string label = "Size: " + std::to_string(area) + " px";
                    putText(image, label, centroid, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1,
                            cv::LINE_AA);

                    //show the picture
                    cv::imshow("Color Tracking", image);
                    */
                }
            }
        }
        if (totalArea > 0) {
            cv::Point2f overallCentroid(static_cast<float>(weightedSumX / totalArea),
                                        static_cast<float>(weightedSumY / totalArea));

            /*
            // Draw the overall centroid
            circle(image, overallCentroid, 10, cv::Scalar(0, 0, 255), -1);
            */

            // Calculate the horizontal difference from the center of the screen
            float screenCenterX = segmented.cols / 2.0f;
            difference = overallCentroid.x - screenCenterX;
            // Normalize to [-1, 1]
            difference = difference / screenCenterX;
        }
        return std::make_pair(difference, isValid);
    }


    catch (std::exception &e) {
        std::cout << "test" << std::endl;
        std::cerr << e.what() << std::endl;
        return std::make_pair(0.0f, false);
    }
}


// Definitions of member functions of DataReceiver
DataReceiver::DataReceiver(const std::string& host, int port)
    : io_service_(), socket_(io_service_), endpoint_(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port)) {
    connect();
}

void DataReceiver::connect() {
    socket_.connect(endpoint_);
}

void DataReceiver::updateData() {
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
            parseData(json_message);
        } else {
            break;
        }
    }
}

double DataReceiver::getBatteryPercentage() const {
    return battery_percentage_;
}

double DataReceiver::getDistanceMm() const {
    return distance_mm_;
}

double DataReceiver::getSpeedY() const {
    return speed_y_;
}

void DataReceiver::parseData(const std::string& data) {
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
