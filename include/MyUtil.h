/*

author: chenjinbo  date:2022/10/05
*/
#pragma once

#include <opencv2/opencv.hpp>
#include <QImage>

namespace kmod {
	template<typename T>
	static T  limitV(const T v, const T max, const T min)
	{
		T ret = v;
		if (ret < min) ret = min;
		if (ret > max) ret = max;

		return ret;
	};
	class MyUtil{
	public:
		static cv::Mat QImage2cvMat(const QImage& image);
		static QImage  MatToQImage(const cv::Mat& mat);
		static int64_t GetTimeStamp();
		static std::string Gettmstr(long long timestamp);
		static std::string GettmstrFile(long long timestamp);		
		static std::tm* Gettm(long long timestamp);
		static bool pathExist(std::string sFullPath);
		static cv::Mat getDefaultMat();
		static std::string getExePath();
		
	};	
}

