#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <regex>

// 前処理とクロップを行う関数
cv::Mat preprocessAndCrop(const cv::Mat& image, int x, int y, int width, int height) {
    // グレースケール
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // ROIでクロップ
    cv::Rect roi(x, y, width, height);
    cv::Mat cropped = gray(roi);

    // ぼかし＋しきい値処理
    cv::Mat blurred, thresh;
    cv::GaussianBlur(cropped, blurred, cv::Size(3, 3), 0);
    cv::threshold(blurred, thresh, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    return thresh;
}

// OCRで数字を抽出する関数
std::string extractNumberFromImage(const cv::Mat& thresh) {
    tesseract::TessBaseAPI tess;
    if (tess.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY)) {
        throw std::runtime_error("Tesseract初期化に失敗しました");
    }

    tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
    tess.SetVariable("tessedit_char_whitelist", "0123456789.");
    tess.SetImage((uchar*)thresh.data, thresh.cols, thresh.rows, 1, thresh.step);

    std::string text = tess.GetUTF8Text();

    // 正規表現で数字を抽出
    std::regex numberRegex(R"(\d+\.\d+|\d+)");
    std::smatch match;
    if (std::regex_search(text, match, numberRegex)) {
        std::string numberStr = match.str();
        // 「00.49」などの不正フォーマットを検出
        if (std::regex_match(numberStr, std::regex(R"(0\d+\.\d+|0\d+)"))) {
            throw std::invalid_argument("不正な数値形式が検出されました: " + numberStr);
        }

        // doubleに変換
        return std::stod(numberStr);
    }

    throw std::runtime_error("数値が認識できませんでした");
}

int main() {
    // 画像の読み込み
    cv::Mat image = cv::imread("../test/IMG_9.png");
    if (image.empty()) {
        std::cerr << "❌ 画像が読み込めません" << std::endl;
        return -1;
    }

    try {
        cv::Mat thresh = preprocessAndCrop(image, 365, 150, 170, 150);

        double number = extractNumberFromImage(thresh);
        std::cout << "🔢 数字抽出結果: " << number << std::endl;

        cv::imshow("thresh", thresh);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }
    catch (const std::exception& e) {
        std::cerr << "⚠ エラー: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
