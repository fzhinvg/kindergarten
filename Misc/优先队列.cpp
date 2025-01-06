//
// Created by fzhinvg on 2025/1/6.
// [此文件是有缺陷的]
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <vector>

std::mutex mtx;
std::condition_variable cv;
std::priority_queue<std::pair<int, int>> priorityQueue;
std::atomic<int> globalSequence{0};
bool start = false; // 新增变量，控制所有线程的启动

void highPriorityTask()
{
	int seq = globalSequence++;
	{
		std::lock_guard<std::mutex> lock(mtx);
		priorityQueue.emplace(1, seq); // 高优先级，加上唯一序列号
	}
	cv.notify_all();

	std::unique_lock<std::mutex> lock(mtx);
	// 等待主线程的启动通知
	cv.wait(lock, []
	{ return start; });

	// 进入等待队列，直到优先级条件满足
	cv.wait(lock, [seq]
	{
		return !priorityQueue.empty() && priorityQueue.top().second == seq;
	});
	std::cout << "High priority task acquired the mutex" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	priorityQueue.pop();
	cv.notify_all();
}

void lowPriorityTask()
{
	int seq = globalSequence++;
	{
		std::lock_guard<std::mutex> lock(mtx);
		priorityQueue.emplace(0, seq); // 低优先级，加上唯一序列号
	}
	cv.notify_all();

	std::unique_lock<std::mutex> lock(mtx);
	// 等待主线程的启动通知
	cv.wait(lock, []
	{ return start; });

	// 进入等待队列，直到优先级条件满足
	cv.wait(lock, [seq]
	{
		return !priorityQueue.empty() && priorityQueue.top().second == seq;
	});
	std::cout << "Low priority task acquired the mutex" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	priorityQueue.pop();
	cv.notify_all();
}

int main()
{
	std::vector<std::thread> threads;
	threads.emplace_back(highPriorityTask);
	for (int i = 0; i < 15; ++i)
	{
		threads.emplace_back(lowPriorityTask);
	}

	// 模拟主线程的一些初始化工作
//	std::this_thread::sleep_for(std::chrono::seconds(1));

	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, []
		{ return priorityQueue.size() == 16; });
		start = true; // 设置开始标志
	}
	cv.notify_all(); // 通知所有线程可以开始竞争

	for (auto &t: threads)
	{
		t.join();
	}

	return 0;
}
