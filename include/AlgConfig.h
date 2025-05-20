/*
author: chenjinbo  date:2022/10/05
*/
#pragma once
#include <opencv4/opencv2/opencv.hpp>
#include <vector>
#include <mutex>

namespace kmod {
	class AlgConfig
	{

	public:
		AlgConfig() = default;
		bool load(std::string filename);
		void save();		
	public:
		int		AlmSumThresh = 15;
		int		WarnSumThresh = 5;
		int		NoDataSumThresh = 5;
		int		RunSpeed = 4;

		
	private:
		std::string		strfilename;
	
	};
}

