#include <opencv2/opencv.hpp>

// 临近插值法 RGB
cv::Mat NearestInterpolation(const cv::Mat &input, int width, int height) {
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
	  float src_x = ((float)x) * scale_w;
	  float src_y = ((float)y + 0.5f) * scale_h - 0.5f;

	  // 取最临近的索引
	  int nearest_x = cvRound(src_x);
	  int nearest_y = cvRound(src_y);

	  // 处理越界
	  if (nearest_x >= src_w || nearest_y >= src_h)
		output.at<cv::Vec3b>(y, x) = 0;
	  else
		output.at<cv::Vec3b>(y, x) = input.at<cv::Vec3b>(nearest_y, nearest_x);
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

  // 临近插值 opencv
  cv::Mat img_nearest;
  cv::resize(img, img_nearest, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_NEAREST);
  cv::resize(img_nearest, img_nearest, cv::Size(img_nearest.cols * 2, img_nearest.rows * 2), 0, 0, cv::INTER_NEAREST);

  // 临近插值
  cv::Mat test_nearest = NearestInterpolation(img, img.cols / 2, img.rows / 2);
  test_nearest = NearestInterpolation(test_nearest, test_nearest.cols * 2, test_nearest.rows * 2);

  // 显示结果
  cv::imshow("img", img);
  cv::imshow("img_nearest", img_nearest);
  cv::imshow("test_nearest", test_nearest);

  // ESC 退出
  while (cv::waitKey(0) != 27);
  return 0;
}
