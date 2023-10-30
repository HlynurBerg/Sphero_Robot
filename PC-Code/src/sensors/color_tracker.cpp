#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

// kode tatt fra python og ish direkte oversatt til C++.
//TODO: Fjerne namespace - legge til kommentarer - legge til i Cmake

using namespace std;
using namespace cv;

int main() {
    VideoCapture cap(0);// TODO: Endre denne til å ta inn robot kamera

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    if (!cap.isOpened()) {
        cout << "Error: Couldn't open the camera." << endl;
        return -1;
    }

    int min_contour_area = 500;

    while (true) {
        Mat image, hsv, mask, segmented, gray, edges;
        vector<vector<Point>> contours;
        Scalar lower_bound(13, 175, 50);
        Scalar upper_bound(23, 255, 255);

        cap >> image;
        if (image.empty()) {
            cout << "Error: Couldn't read a frame." << endl;
            break;
        }

        cvtColor(image, hsv, COLOR_BGR2HSV);

        inRange(hsv, lower_bound, upper_bound, mask);
        bitwise_and(image, image, segmented, mask);

        cvtColor(segmented, gray, COLOR_BGR2GRAY);

        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            if (contourArea(contour) > min_contour_area) {
                Moments M = moments(contour);
                Point2f centroid; //TODO centroid har x & y koordinater. Bruk for "follow me" mode
                if (M.m00 != 0) {
                    centroid = Point2f(static_cast<float>(M.m10 / M.m00), static_cast<float>(M.m01 / M.m00));
                } else {
                    centroid = Point2f(0, 0);
                }

                circle(image, centroid, 5, Scalar(255, 0, 0), -1);

                string label = "Size: " + to_string(contourArea(contour)) + " px";
                putText(image, label, centroid, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1, LINE_AA);

            }
        }

        imshow("Original Image", image);

        if (waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}