#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

// kode tatt fra python og ish direkte oversatt til C++.
//TODO: Fjerne namespace - legge til kommentarer - legge til i Cmake

using namespace std;
using namespace cv;

int main() {
    // Open the camera
    VideoCapture cap(0);// TODO: Endre denne til å ta inn robot kamera

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    if (!cap.isOpened()) {
        cout << "Error: Couldn't open the camera." << endl;
        return -1;
    }

    int min_contour_area = 500;

    // Define the range of the color we are looking for
    Scalar lower_bound(13, 175, 50);
    Scalar upper_bound(23, 255, 255);
    Mat image, hsv, mask, segmented, gray, edges;
    vector<vector<Point>> contours;

    while (true) {

        cap >> image;
        if (image.empty()) {
            cout << "Error: Couldn't read a frame." << endl;
            break;
        }

        // Turn image into hsv
        cvtColor(image, hsv, COLOR_BGR2HSV);

        // Find color in predefined range
        inRange(hsv, lower_bound, upper_bound, mask);
        bitwise_and(image, image, segmented, mask);

        // Find contours
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        double weightedSumX = 0.0, weightedSumY = 0.0, totalArea = 0.0;

        // Go through all contours found
        for (const auto& contour : contours) {
            double area = contourArea(contour);
            if (area > min_contour_area) {
                Moments M = moments(contour);
                Point2f centroid;

                if (M.m00 != 0) {
                    centroid = Point2f(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));

                    // Add to the weighted sum and total area
                    weightedSumX += centroid.x * area;
                    weightedSumY += centroid.y * area;
                    totalArea += area;
                }

                //Draw a circle on each contour's centroid and display the size
                circle(image, centroid, 5, Scalar(255, 0, 0), -1);
                string label = "Size: " + to_string(area) + " px";
                putText(image, label, centroid, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1, LINE_AA);
            }
        }

        // Avoid division by zero
        if (totalArea > 0) {
            Point2f overallCentroid = Point2f(static_cast<float>(weightedSumX / totalArea),
                                              static_cast<float>(weightedSumY / totalArea));

            // Draw a single circle at the weighted centroid of all contours
            circle(image, overallCentroid, 10, Scalar(0, 0, 255), -1);
        }

        // Display the resulting image
        imshow("Contours", image);

        if (waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}