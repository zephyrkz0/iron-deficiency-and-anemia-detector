#include <opencv2/opencv.hpp>
#include <windows.h>
#include <commdlg.h>
#include <iostream>

std::string openFileDialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return std::string(filename);
    else return "";
}

int main() {
    std::string imagePath = openFileDialog();
    if (imagePath.empty()) return -1;

    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) return -1;

    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    cv::Mat red = channels[2];

    cv::GaussianBlur(red, red, cv::Size(7, 7), 0);
    cv::Mat thresh;
    cv::threshold(red, thresh, 120, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxAvg = -1;
    cv::Rect bestRect;

    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        if (rect.area() < 300) continue;
        cv::Mat roi = red(rect);
        double avg = cv::mean(roi)[0];
        if (avg > maxAvg) {
            maxAvg = avg;
            bestRect = rect;
        }
    }

    cv::rectangle(image, bestRect, cv::Scalar(0, 255, 0), 2);
    std::cout << "Max localized red region avg: " << maxAvg << std::endl;
    if (maxAvg < 180) std::cout << "Possible iron deficiency" << std::endl;
    else std::cout << "Normal" << std::endl;

    cv::imshow("Localized Red Region", image);
    cv::waitKey(0);
    return 0;
}
