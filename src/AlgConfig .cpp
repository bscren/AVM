#include "AlgConfig.h"
namespace kmod {
	bool AlgConfig::load(std::string filename)	{

		strfilename = filename;
		cv::FileStorage store(strfilename, cv::FileStorage::READ);
		if (!store.isOpened()) return false;
		cv::FileNode n1;
		
		n1 = store["AlmSumThresh"]; cv::read(n1, AlmSumThresh, 20);
		n1 = store["WarnSumThresh"]; cv::read(n1, WarnSumThresh, 10);
		n1 = store["NoDataSumThresh"]; cv::read(n1, NoDataSumThresh, 5);
		n1 = store["RunSpeed"]; cv::read(n1, RunSpeed, 4);



		store.release();
		return true;
	}
	void AlgConfig::save()
	{
		cv::FileStorage fs1(strfilename, cv::FileStorage::WRITE);
		
		cv::write(fs1, "AlmSumThresh", AlmSumThresh);
		cv::write(fs1, "WarnSumThresh", WarnSumThresh);
		cv::write(fs1, "NoDataSumThresh", NoDataSumThresh);
		cv::write(fs1, "RunSpeed", RunSpeed);
		fs1.release();
	}
}
