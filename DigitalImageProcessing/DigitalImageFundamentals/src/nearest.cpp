#include <iostream>
#include <opencv2/opencv.hpp>

// 临近插值法
cv::Mat NearestNeighbourInterpolation(const cv::Mat &input, int scale_width, int scale_height) {
  int new_cols = scale_width;
  int new_rows = scale_height;

  float scale_x = float(new_cols) / float(input.cols);
  float scale_y = float(new_rows) / float(input.rows);

  int channels = input.channels();

  cv::Mat output(new_rows, new_cols, input.type());
  for (int i = 0; i < new_rows; i++) {
	for (int j = 0; j < new_cols; j++) {
	  int x = cvRound((float)i / scale_x);
	  int y = cvRound((float)j / scale_y);
	  if (x < 0)
		x = 0;
	  if (y < 0)
		y = 0;
	  if (x >= input.cols)
		x = input.cols - 1;
	  if (y > input.rows)
		y = input.rows - 1;
	  for (int c = 0; c < channels; c++) {
		output.at<cv::Vec3b>(i, j)[c] = input.at<cv::Vec3b>(x, y)[c];
	  }
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
  std::cout << img.cols << std::endl;
  std::cout << img.rows << std::endl;

  // 临近插值
  cv::Mat img_nearest;
  cv::resize(img, img_nearest, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_NEAREST);
  cv::resize(img_nearest, img_nearest, cv::Size(img_nearest.cols * 2, img_nearest.rows * 2), 0, 0, cv::INTER_NEAREST);
  // 双线性
  cv::Mat img_linear;
  resize(img, img_linear, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_LINEAR);
  resize(img_linear, img_linear, cv::Size(img_linear.cols * 2, img_linear.rows * 2), 0, 0, cv::INTER_LINEAR);
  // 双立方
  cv::Mat img_cubic;
  resize(img, img_cubic, cv::Size(img.cols / 2, img.rows / 2), 0, 0, cv::INTER_CUBIC);
  resize(img_cubic, img_cubic, cv::Size(img_cubic.cols * 2, img_cubic.rows * 2), 0, 0, cv::INTER_CUBIC);

  // 临近插值
  cv::Mat test_nearest = NearestNeighbourInterpolation(img, img.cols / 2, img.rows / 2);
  test_nearest = NearestNeighbourInterpolation(test_nearest, test_nearest.cols * 2, test_nearest.rows * 2);

  cv::imshow("img", img);
  cv::imshow("img_nearest", img_nearest);
  cv::imshow("img_linear", img_linear);
  cv::imshow("img_cubic", img_cubic);
  cv::imshow("test_nearest", test_nearest);

  // ESC 退出
  while (cv::waitKey(0) != 27);

  return 0;
}
