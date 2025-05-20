/*
author: chenjinbo  date:2022/10/05
*/
#pragma once
/*#include <time.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <memory>
#include <mutex>*/

#include <Util/util.h>
#include <string>
#include <SCL/SCL.hpp>

namespace kmod {

	class KConfig :public toolkit::noncopyable
    {
		
	public:
		static KConfig &Instance() {
			static KConfig inst;
			return inst;
		}
        void setcfgfile( std::string name);
		void closecfgfile();
		scl::config_file* cfg() { return cfgfile; };
		KConfig() {};
		~KConfig() {};
	public:
		scl::config_file*  cfgfile;		
	};
}
