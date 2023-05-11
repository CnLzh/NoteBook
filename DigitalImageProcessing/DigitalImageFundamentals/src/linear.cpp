#include <opencv2/opencv.hpp>
#include <cmath>

cv::Mat LinearInterpolation(cv::Mat &input, int width, int height) {
  // 输入图像的宽高和通道数
  int src_w = input.cols;
  int src_h = input.rows;
  int channels = input.channels();

  // 创建输出图像
  cv::Mat output(height, width, input.type());
  int dst_w = output.cols;
  int dst_h = output.rows;

  // 计算宽高缩放比例
  float scale_w = float(src_w) / float(dst_w);
  float scale_h = float(src_h) / float(dst_h);

  for (int y = 0; y < dst_h; y++) {
	for (int x = 0; x < dst_w; x++) {

	}
  }
}

int main() {
  cv::Mat img = cv::imread("../../images/clock.jpg", cv::IMREAD_COLOR);
  if (img.empty()) {
	std::cout << "Failed open image." << std::endl;
	return -1;
  }

  // 双线性插值 opencv
  cv::Mat img_linear;
  resize(img, img_linear, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_LINEAR);
  resize(img_linear, img_linear, cv::Size(img_linear.cols * 2, img_linear.rows * 2), 0, 0, cv::INTER_LINEAR);

  // 显示结果
  cv::imshow("img", img);
  cv::imshow("img_nearest", img_linear);

  while (cv::waitKey(0) != 27);
  return 0;
}