
/*
author: chenjinbo  date:2022/10/05
*/
#pragma once
#include <list>
#include <vector>
#include <mutex>
#include <string>
#include <algorithm>
#include <Util/logger.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace toolkit;

namespace kmod {
	template <typename T,int MAX_QUEQUE = 3, int type=0>
	class KMsg{		
	public:
		KMsg() {}
		~KMsg() {}
		//newest frame
		bool push(T& value){
			std::lock_guard<std::mutex> lock(mutex);
			if (data.size() < MAX_QUEQUE){
				data.push_back(value);
				return true;
			}else{
				if (0 == type ) {			
					data.pop_front();
					data.push_back(value);
				}				
				return false;
			}			
		}
		std::shared_ptr<T> pop(){

			std::lock_guard<std::mutex> lock(mutex);
			if (data.empty()) return nullptr;// 
			std::shared_ptr<T> const res(std::make_shared<T>(data.front()));			
			data.pop_front();
			return res;
		}
		void clear(){

			data.clear();
		}
	private:
		std::list<T> data;
		std::mutex mutex;			
	};
	//std::mutex mutex;		
	//std::lock_guard<std::mutex> lock(mutex);
	template <typename T, int MAX_QUEQUE = 3, int type = 0>
	class KVector {
	public:
		KVector() {}
		~KVector() {}
		//newest frame
		bool push(T& value) {
			std::lock_guard<std::mutex> lock(mutex);
			if (data.size() < MAX_QUEQUE) {
				data.push_back(value);
				return true;

			}
			else {
				if (0 == type) {
					data.erase(std::begin(data));
					data.push_back(value);
				}
				return false;
			}
		}

		bool push_front(T& value) {
			std::lock_guard<std::mutex> lock(mutex);
			if (data.size() < MAX_QUEQUE) {
				data.insert(data.begin(), value);
				return true;

			}
			else {
				if (0 == type) {
					data.pop_back();
					data.insert(data.begin(), value);
				}
				return false;
			}
		}

		std::shared_ptr<T> pop() {
			std::lock_guard<std::mutex> lock(mutex);
			if (data.empty()) return nullptr;// 
			std::shared_ptr<T> const res(std::make_shared<T>(data.front()));
			//TraceL << "prepop size:" << data.size();// << " " << data.front();
			data.erase(std::begin(data));			
			return res;
		}
		std::vector<T> dat() {
			std::lock_guard<std::mutex> lock(mutex);
			return data;
		}
		size_t  size() {
			return data.size();
		}
		void clear()
		{
			data.clear();
		}
	private:
		std::vector<T> data;
		std::mutex mutex;
	};

	typedef struct __MatwithTimer{
		cv::Mat  	imgmat;
		long long 		timestamp;
		//std::string		strtimestamp;
		int			channel;
	}MatTimeStamp;

	typedef struct __Alarm_Data{
		int			id;
		long long 	timestamp;
		std::string	strtimestamp;
		int			channel;
		int			type; ///1000��2000,3000 camera id prefix
		double		position;
		std::string	detial;
		std::string strimgfile;
	}Alarm_Data;


	//max size 3 ,type=0 pop oldest
	extern KMsg<MatTimeStamp>	g_ImgbufList[10];


}

