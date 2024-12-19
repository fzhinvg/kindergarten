//
// Created by fzhinvg on 2024/12/17.
//
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

template<typename Func_t, typename... Args_t>
void measureExecutionTime(Func_t func, Args_t &&... args) // refactor rename RTT? -> runtime timer 意义不大好像
{
	auto start = std::chrono::high_resolution_clock::now();
	func(std::forward<Args_t>(args)...); // 完美
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start; // 时间的数值部分用double存储,单位制用ms
	std::cout << "Function executed in " << duration.count() << " ms" << std::endl;
}

// 示例函数
void exampleFunction(int n)
{
	int sum = 0;
	for (int i = 0; i < n; ++i)
	{
		sum += i;
	}
}

void threadFunction(int n)
{
	int sum = 0;
	for (int i = 0; i < n; ++i)
	{
		sum += i;
	}
}


int main()
{
	int regular_args = 1000000;
	int &lambda_args = regular_args;
	int &thread_args = regular_args;

	measureExecutionTime(exampleFunction, regular_args); // function

	measureExecutionTime([](int n)
						 {
							 int sum = 0;
							 for (int i = 0; i < n; ++i)
							 {
								 sum += i;
							 }
						 }, lambda_args); // lambda

	measureExecutionTime([](int n)
						 {
							 std::thread task(threadFunction, n);
							 task.join();
						 }, thread_args); // thread

	return 0;
}


