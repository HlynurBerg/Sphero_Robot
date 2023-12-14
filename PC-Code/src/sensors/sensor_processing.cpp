#include <sensors/sensor_processing.hpp>

std::pair<float, bool> ColorTracker(cv::Mat image) {
    try {
        //TODO: Define color bounds as a variable, let used decide
        cv::Scalar lower_bound(10, 150, 50);
        cv::Scalar upper_bound(25, 255, 255);
        // Define smallest detected object
        int min_contour_area = 250;
        bool is_valid = false;
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

        double weighted_sum_x = 0.0, weighted_sum_y = 0.0, total_area = 0.0;

        for (const auto &contour: contours) {
            double area = contourArea(contour);
            if (area > min_contour_area) {
                is_valid = true; // Valid object found
                cv::Moments m_moments = moments(contour);
                if (m_moments.m00 != 0) {
                    cv::Point2f centroid(static_cast<float>(m_moments.m10 / m_moments.m00), static_cast<float>(m_moments.m01 / m_moments.m00));
                    weighted_sum_x += centroid.x * area;
                    weighted_sum_y += centroid.y * area;
                    total_area += area;
                }
            }
        }
        if (total_area > 0) {
            cv::Point2f overall_centroid(static_cast<float>(weighted_sum_x / total_area),
                                        static_cast<float>(weighted_sum_y / total_area));

            /*
            // Uncomment this to draw the overall centroid. Good for testing if it works!
            circle(image, overall_centroid, 10, cv::Scalar(0, 0, 255), -1);
            cv::imshow("Color Tracking", image);
            */

            // Calculate the horizontal difference from the center of the screen
            float screen_center_x = segmented.cols / 2.0f;
            difference = overall_centroid.x - screen_center_x;
            // Normalize to [-1, 1]
            difference = difference / screen_center_x;
        }
        return std::make_pair(difference, is_valid);
    }


    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return std::make_pair(0.0f, false);
    }
}


// Definitions of member functions of DataReceiver
DataReceiver::DataReceiver(const std::string& host, int port)
    : io_service_(), socket_(io_service_), endpoint_(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port)) {
    connect();
}

void DataReceiver::connect() { //TODO: capitalizing this name breaks everything. Maybe change name?
    socket_.connect(endpoint_);
}

void DataReceiver::UpdateData() {
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

double DataReceiver::GetBatteryPercentage() const {
    return battery_percentage_;
}

double DataReceiver::GetDistanceMm() const {
    return distance_mm_;
}

double DataReceiver::GetSpeedY() const {
    return speed_y_;
}

void DataReceiver::ParseData(const std::string& data) {
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