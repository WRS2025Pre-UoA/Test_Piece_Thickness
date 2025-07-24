#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <regex>

int main() {
    std::string path = "../thicknessImage/IMG_8.png";
    // 画像の読み込み
    cv::Mat image = cv::imread(path);
    if (image.empty()) {
        std::cerr << "画像が読み込めません" << std::endl;
        return -1;
    }

    // グレースケール変換
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // 数字部分のクロップ
    cv::Rect roi(60, 15, 220, 55);  // (x, y, width, height)
    cv::Mat cropped = gray(roi);

    // 前処理（ぼかし + しきい値）
    cv::Mat blurred, thresh;
    cv::GaussianBlur(cropped, blurred, cv::Size(3, 3), 0);
    cv::threshold(blurred, thresh, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    // Tesseract OCRの初期化
    tesseract::TessBaseAPI tess;
    if (tess.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY)) {
        std::cerr << "Tesseract初期化に失敗しました" << std::endl;
        return -1;
    }

    tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);  // --psm 6
    tess.SetVariable("tessedit_char_whitelist", "0123456789.");

    tess.SetImage((uchar*)thresh.data, thresh.cols, thresh.rows, 1, thresh.step);
    std::string outText = tess.GetUTF8Text();

    // 正規表現で数字抽出
    std::regex numberRegex(R"(\d+\.\d+|\d+)");
    std::smatch match;
    if (std::regex_search(outText, match, numberRegex)) {
        std::cout << "🔢 数字抽出結果: " << match.str() << std::endl;
    } else {
        std::cout << "❌ 認識できた数字が見つかりませんでした" << std::endl;
    }

    // 表示
    cv::imshow("cropped", cropped);
    cv::imshow("thresh", thresh);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
