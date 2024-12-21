//
// Created by fzhinvg on 2024/12/20.
//
#include <algorithm>
#include <random>
#include <vector>
#include <iostream>
#include <string>
#include <functional>
#include <thread>

using std::string;

uint32_t generateSeed(const std::string &seedString)
{
	static std::hash<std::string> hash_maker;
	return hash_maker(seedString);
}

void runAtFixedTime(const std::function<void()>& func, const std::chrono::system_clock::time_point& time) {
	// 计算当前时间到目标时间的差值
	auto now = std::chrono::system_clock::now();
	if (time > now) {
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time - now);
		std::this_thread::sleep_for(duration);
	}
	// 运行函数
	func();
}


int main()
{
	string inputSeed = "All Your Heart";
	auto seed = generateSeed(inputSeed);
//	std::random_device random_device; // token 是指定随机数设备用的

	std::mt19937 generator(seed);

	std::vector<int> target_vector{0, 1, 2, 3, 4, 5, 6, 7};

	std::shuffle(target_vector.begin(), target_vector.end(), generator);
	// 听说用的是Fisher-Yates

	for (auto &item: target_vector)
	{
		std::cout << item << std::endl;
	}

	std::uniform_int_distribution<int> int_distribution(0, 100);

	int random_int = int_distribution(generator);

#if 0
	int inside_circle = 0;
	const int num_samples = 4000000;
	std::uniform_real_distribution<double> dist(0.0, 1.0);
	// 为什么我感觉浮点计算因为优化,导致速度比整型还快
	// 蒙特卡罗模拟的收敛速度与模拟次数的平方根成反比
	// 为了将误差减半，需要增加四倍的模拟次数
	for (int i = 0; i < num_samples; ++i)
	{
		double x = dist(generator);
		double y = dist(generator);
		if (x * x + y * y <= 1.0)
		{
			++inside_circle;
		}
	}
	double pi_estimate = 4.0 * inside_circle / num_samples;

	std::cout << pi_estimate << std::endl;
#endif


	auto target_time=std::chrono::system_clock::now()+std::chrono::seconds(2);
	runAtFixedTime([]{std::cout<<"i like spd gar";},target_time);



	return 0;
}//一个新想法诞生

#if 0
#include <functional>
#include <chrono>
#include <queue>
#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include <random>
#include <string>

struct Event
{
	std::chrono::system_clock::time_point execution_time;
	std::function<void()> task;

	bool operator>(const Event &other) const
	{
		return this->execution_time > other.execution_time;
	}
};

class EventManager
{
private:
	std::priority_queue<Event, std::vector<Event>, std::greater<>> _event_queue;
public:
	void pollingRun()
	{
		std::chrono::system_clock::time_point now;
		std::chrono::seconds poll_time{1};

		while (!_event_queue.empty())
		{
			now = std::chrono::system_clock::now();
			if (now > _event_queue.top().execution_time)
			{
				_event_queue.top().task();
				_event_queue.pop();
			}
			std::this_thread::sleep_for(poll_time);
		}
	}

	void run()
	{
		std::chrono::system_clock::time_point now;

		while (!_event_queue.empty())
		{
			std::this_thread::sleep_until(_event_queue.top().execution_time);
			_event_queue.top().task();
			_event_queue.pop();
		}
	}

	void addEvent(const Event &event)
	{
		_event_queue.emplace(event);
	}
};

void fakeTask()
{
	static int task_count = 0;
	auto now = std::chrono::system_clock::now();
	std::time_t local_time = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm = *std::localtime(&local_time);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_tm);
	std::cout << "fake task " << task_count << " called at " << buffer << std::endl;
	++task_count;
}

uint32_t generateSeed(const std::string &seed_string)
{
	static std::hash<std::string> hash_maker;
	return hash_maker(seed_string);
}

int main()
{
//	std::chrono::system_clock::time_point target_time = std::chrono::system_clock::now();
//	EventManager eventManager;
//	auto offset = std::chrono::seconds(5);
//	eventManager.addEvent({target_time, fakeTask});
//	eventManager.addEvent({target_time + offset, []
//	{ std::cout << "fall for love"; }});
//	eventManager.addEvent({target_time + 2 * offset, fakeTask});
//	eventManager.addEvent({target_time + 3 * offset, fakeTask});
//
//	eventManager.run();

	std::string inputSeed = "All Your Heart";
	auto seed = generateSeed(inputSeed);
	std::mt19937 generator(seed);
	std::normal_distribution<double> distribution(30.0, 10.0); // 均值30秒，标准差10秒


	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point end_time = start_time + std::chrono::seconds(60);


	EventManager eventManager;
	std::vector<int> histogram(6, 0); // 用于统计每10秒内的事件数量

	for (int i = 0; i < 1000; ++i)
	{
		double random_seconds = distribution(generator);
		auto event_time = start_time + std::chrono::seconds(static_cast<int>(random_seconds));

		// 确保事件时间在有效范围内
		if (event_time > end_time)
		{
			event_time = end_time;
		}

		eventManager.addEvent({event_time, fakeTask});

		// 统计事件
		int index = static_cast<int>(random_seconds / 10);
		if (index < 6)
		{
			histogram[index]++;
		}
	}

	eventManager.run();

	for (int i = 0; i < 6; ++i)
	{
		std::cout << "Events in interval [" << i * 10 << ", " << (i + 1) * 10 << "): " << histogram[i] << std::endl;
	}
	// 当然,这是现有分布再执行
	// 也可以写一个概率函数,然他在某个时间段的发生概率更大,有更大可能性返回一个bool true
	// 我可能把这个想象的有点太简单,那个体量不太适合写在这里了

	return 0;
}
#endif

