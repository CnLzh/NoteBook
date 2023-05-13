#include <opencv2/opencv.hpp>

// 计算权重
float CubicWeight(float x) {
  const float coefficient = 0.5f;
  float abs_x = std::abs(x);
  if (abs_x <= 1) {
	return (float)((coefficient + 2) * std::pow(abs_x, 3)
		- (coefficient + 3) * std::pow(abs_x, 2) + 1);
  } else if (abs_x < 2) {
	return (float)(coefficient * std::pow(abs_x, 3)
		- 5 * coefficient * std::pow(abs_x, 2)
		+ 8 * coefficient * abs_x - 4 * coefficient);
  } else {
	return 0;
  }
}

// 另一种计算权重的方式
std::vector<float> CubicWeightVec4(float x) {
  const float A = -0.75;
  std::vector<float> vec(4);
  vec[0] = ((A * (x + 1) - 5 * A) * (x + 1) + 8 * A) * (x + 1) - 4 * A;
  vec[1] = ((A + 2) * x - (A + 3)) * x * x + 1;
  vec[2] = ((A + 2) * (1 - x) - (A + 3)) * (1 - x) * (1 - x) + 1;
  vec[3] = 1.f - vec[0] - vec[1] - vec[2];
  return vec;
}

// 双立方插值 效果不太好，暂时没理解opencv的实现方式
cv::Mat CubicInterpolation(cv::Mat &input, int width, int height) {
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

	  // 取临近十六个点的索引
	  int x0 = int(floor(src_x));
	  int y0 = int(floor(src_y));

#if 1
	  // 计算水平方向权重
	  std::vector<float> wx(4);
	  for (int i = 0; i < 4; i++) {
		float dx = src_x - (float)(x0 + i);
		wx[i] = CubicWeight(dx);
	  }

	  // 计算垂直方向权重
	  std::vector<float> wy(4);
	  for (int i = 0; i < 4; i++) {
		float dy = src_y - (float)(y0 + i);
		wy[i] = CubicWeight(dy);
	  }
#endif

#if 0
	  float dx = src_x - (float)x0;
	  float dy = src_y - (float)y0;

	  std::vector<float> wx = CubicWeightVec4(dx);
	  std::vector<float> wy = CubicWeightVec4(dy);
#endif

#if 1
	  x0 = std::max(0, std::min(x0, src_w - 1));
	  y0 = std::max(0, std::min(y0, src_h - 1));
#endif
	  // 计算像素点的值
	  cv::Vec3f result{0.0f, 0.0f, 0.0f};
	  for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
		  int px = std::max(0, std::min(x0 + i - 1, src_w - 1));
		  int py = std::max(0, std::min(y0 + j - 1, src_h - 1));
		  cv::Vec3b pixel = input.at<cv::Vec3b>(py, px);
		  float weight = wx[i] * wy[j];
		  result += weight * cv::Vec3f(pixel[0], pixel[1], pixel[2]);
		}
	  }

	  output.at<cv::Vec3b>(y, x) =
		  cv::Vec3b(cvRound(result[0]), cvRound(result[1]), cvRound(result[2]));

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

  // 双立方插值 opencv
  cv::Mat img_cubic;
  resize(img, img_cubic, cv::Size(img.cols * 2, img.rows * 2), 0, 0, cv::INTER_CUBIC);
  resize(img_cubic, img_cubic, cv::Size(img_cubic.cols * 2, img_cubic.rows * 2), 0, 0, cv::INTER_CUBIC);

  // 双立方插值
  cv::Mat test_cubic;
  test_cubic = CubicInterpolation(img, img.cols * 2, img.rows * 2);
  test_cubic = CubicInterpolation(test_cubic, test_cubic.cols * 2, test_cubic.rows * 2);

  // 显示结果
  cv::imshow("img", img);
  cv::imshow("img_cubic", img_cubic);
  cv::imshow("test_cubic", test_cubic);

  while (cv::waitKey(0) != 27);
  return 0;
}