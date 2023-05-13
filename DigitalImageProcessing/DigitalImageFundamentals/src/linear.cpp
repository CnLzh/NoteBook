#include <opencv2/opencv.hpp>
#include <cmath>

// 双线性插值
cv::Mat LinearInterpolation(cv::Mat &input, int width, int height) {
  // 输入图像的宽高和通道数
  int src_w = input.cols;
  int src_h = input.rows;

  // 创建输出图像
  cv::Mat output(height, width, input.type());
  int dst_w = output.cols;
  int dst_h = output.rows;

  // 计算宽高缩放比例
  float scale_w = float(src_w) / float(dst_w);
  float scale_h = float(src_h) / float(dst_h);

  for (int y = 0; y < dst_h; y++) {
	for (int x = 0; x < dst_w; x++) {
	  // 当前点在源图像中的几何中心位置
	  float src_x = ((float)x + 0.5f) * scale_w - 0.5f;
	  float src_y = ((float)y + 0.5f) * scale_h - 0.5f;

	  // 取临近四个点的索引
	  int x0 = floor(src_x);
	  int y0 = floor(src_y);
	  int x1 = x0 + 1;
	  int y1 = y0 + 1;

	  // 计算权重
	  float w00 = ((float)x1 - src_x) * ((float)y1 - src_y);
	  float w10 = (src_x - (float)x0) * ((float)y1 - src_y);
	  float w01 = ((float)x1 - src_x) * (src_y - (float)y0);
	  float w11 = (src_x - (float)x0) * (src_y - (float)y0);

	  // 处理越界
	  x0 = std::max(0, std::min(x0, src_w - 1));
	  x1 = std::max(0, std::min(x1, src_w - 1));
	  y0 = std::max(0, std::min(y0, src_h - 1));
	  y1 = std::max(0, std::min(y1, src_h - 1));

	  // 计算像素点的值
	  output.at<cv::Vec3b>(y, x) = input.at<cv::Vec3b>(y0, x0) * w00
		  + input.at<cv::Vec3b>(y0, x1) * w10
		  + input.at<cv::Vec3b>(y1, x0) * w01
		  + input.at<cv::Vec3b>(y1, x1) * w11;
	}
  }
  return output;
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

  cv::Mat test_linear;
  test_linear = LinearInterpolation(img, img.cols / 2, img.rows / 2);
  test_linear = LinearInterpolation(test_linear, test_linear.cols * 2, test_linear.rows * 2);

  // 显示结果
  cv::imshow("img", img);
  cv::imshow("img_linear", img_linear);
  cv::imshow("test_linear", test_linear);

  while (cv::waitKey(0) != 27);
  return 0;
}