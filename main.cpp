#include <opencv2/opencv.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>

cv::Mat chromaLibrary(const cv::Mat& cat, const cv::Mat& background,
                      const cv::Scalar& lower, const cv::Scalar& upper) {
    cv::Mat mask, result = cat.clone();
    cv::inRange(cat, lower, upper, mask);
    background.copyTo(result, mask);
    return result;
}

cv::Mat chromaNative(const cv::Mat& cat, const cv::Mat& background,
                     const cv::Scalar& lower, const cv::Scalar& upper) {
    cv::Mat result(cat.size(), CV_8UC3);
    for (int y = 0; y < cat.rows; ++y) {
        for (int x = 0; x < cat.cols; ++x) {
            cv::Vec3b pixel = cat.at<cv::Vec3b>(y, x);
            bool selected = true;
            for (int c = 0; c < 3; ++c)
                selected = selected && lower[c] <= pixel[c] && pixel[c] <= upper[c];
            result.at<cv::Vec3b>(y, x) = selected
                ? background.at<cv::Vec3b>(y, x) : pixel;
        }
    }
    return result;
}

int main() {
    cv::Mat cat = cv::imread("examples/cat.jpg");
    cv::Mat background = cv::imread("examples/background.jpg");
    if (cat.empty() || background.empty()) {
        std::cout << "Не удалось открыть картинки. Запустите программу из папки проекта.\n";
        return 1;
    }
    cv::resize(background, background, cat.size());

    int low[3], high[3];
    std::cout << "Нижняя граница RGB (например, 0 180 0): ";
    if (!(std::cin >> low[0] >> low[1] >> low[2])) return 1;
    std::cout << "Верхняя граница RGB (например, 100 255 100): ";
    if (!(std::cin >> high[0] >> high[1] >> high[2])) return 1;
    for (int c = 0; c < 3; ++c) {
        if (low[c] < 0 || high[c] > 255 || low[c] > high[c]) {
            std::cerr << "Нужно: 0 <= нижняя граница <= верхняя граница <= 255.\n";
            return 1;
        }
    }
    cv::Scalar lower(low[2], low[1], low[0]);
    cv::Scalar upper(high[2], high[1], high[0]);
    cv::setNumThreads(1);

    cv::Mat library = chromaLibrary(cat, background, lower, upper);
    cv::Mat native = chromaNative(cat, background, lower, upper);
    const int repeats = 100;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < repeats; ++i)
        library = chromaLibrary(cat, background, lower, upper);
    double libraryMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start).count() / repeats;

    start = std::chrono::steady_clock::now();
    for (int i = 0; i < repeats; ++i)
        native = chromaNative(cat, background, lower, upper);
    double nativeMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start).count() / repeats;

    if (cv::norm(library, native, cv::NORM_INF) != 0) {
        std::cerr << "Результаты не совпадают.\n";
        return 1;
    }
    if (!cv::imwrite("examples/result_opencv.png", library) ||
        !cv::imwrite("examples/result_native.png", native)) {
        std::cerr << "Не удалось сохранить результат.\n";
        return 1;
    }
    std::cout << std::fixed << std::setprecision(4)
              << "OpenCV: " << libraryMs << " мс\n"
              << "Нативная версия: " << nativeMs << " мс\n"
              << "Нативная / OpenCV: " << nativeMs / libraryMs << '\n'
              << "Результаты совпадают и сохранены в examples/.\n";
    return 0;
}
