#include <opencv2/opencv.hpp>

int main() {
  cv::Mat img = cv::imread("../../images/clock.jpg", cv::IMREAD_COLOR);
  if (img.empty()) {
	std::cout << "Failed open image." << std::endl;
	return -1;
  }

  // 双立方插值 opencv
  cv::Mat img_cubic;
  resize(img, img_cubic, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_CUBIC);
  resize(img_cubic, img_cubic, cv::Size(img_cubic.cols * 2, img_cubic.rows * 2), 0, 0, cv::INTER_CUBIC);

  // 显示结果
  cv::imshow("img", img);
  cv::imshow("img_nearest", img_cubic);

  while (cv::waitKey(0) != 27);
  return 0;
}