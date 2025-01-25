//
// Created by fzhinvg on 2025/1/13.
// 至于为什么不叫utils或者utilities,是因为它们不好念

#ifndef TC_KIT_H
#define TC_KIT_H

#include <functional>
#include <iostream>
#include <chrono>
//#include <ctime>
#include <iomanip>

namespace tc // 哦,我的老天.这样简直酷到爆炸
{
	// 测量运行时间
	template<typename Func_t, typename... Args_t>
	void measureExecutionTime(Func_t func, Args_t &&... args)
	{
		auto start = std::chrono::high_resolution_clock::now();
		func(std::forward<Args_t>(args)...);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start; // 时间的数值部分用double存储,单位制用ms
		std::cout << "Function executed in " << duration.count() << " ms" << std::endl;
	}

	// 时间戳
	void timestamp(const std::string &message = "")
	{
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::cout << "[" << std::put_time(std::localtime(&in_time_t), "%H:%M:%S") // 时间模板年月日部分: fmt: %Y-%m-%d
				  //<< "." << std::setfill('0') << std::setw(3) << ms.count()  // 毫秒位
				  << "] " << message << " ";//<< std::endl; // 是否换行
	}

}


#endif //TC_KIT_H
