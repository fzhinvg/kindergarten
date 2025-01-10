//
// Created by fzhinvg on 2025/1/10.
//
#define THREAD_DEMO false

#include <iostream>
#include <omp.h> // openmp需要编译器支持
/* CMakeLists:
 * find_package(OpenMP REQUIRED)
 * target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
 * target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp  -g -O0)
 * */
int main()
{
	// 设置并行线程数
	omp_set_num_threads(4);

	// 并行化 for 循环
#pragma omp parallel for default(none) shared(std::cout) // 添加 default(none) 子句并指定共享变量
	for (int i = 0; i < 10; ++i)
	{
		int thread_id = omp_get_thread_num();
#pragma omp critical // 使用 OpenMP 的 critical 指令确保一次只有一个线程可以执行临界区内的代码
		{
			std::cout << "Thread " << thread_id << " is processing iteration " << i << '\n';
		}
	}
	//也就是说openmp只是一套并行标准,而cpp有自己的并行实现方案,就是thread以及相关文件组成的多线程方案.
	return 0;
}
/*
 * omp的0123只是表面上的分配的线程id是0123
 * 实际上是thread的版本更加准确
 * 因为不存在线程复用
 * 而是生成了新线程
 * 但是在omp中它们严格的按照0123...的顺序被命名
 * ↑我原本是这么以为的,但是实际上好像是↓
 * openmp自带一个线程池管理线程复用
 * 而cpp thread,需要自己手动实现线程池来实现线程复用
 * */
// 我当前的环境好像并不允许我使用execution头文件中的运行策略,并行算法的参数提示也不能被clion正确显示
#if THREAD_DEMO
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex mtx;

void worker(int i)
{
	size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
	std::lock_guard<std::mutex> lock(mtx);
	std::cout << "Thread " << thread_id << " is processing iteration " << i << '\n';
}

int main()
{
	// 设置并行线程数
	int num_threads = 4;
	int num_iterations = 10;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_iterations; ++i)
	{
		threads.emplace_back(worker, i);
		if (threads.size() >= num_threads)
		{
			for (auto &t: threads)
			{
				t.join();
			}
			threads.clear();
		}
	}

	// 确保所有线程都完成
	for (auto &t: threads)
	{
		t.join();
	}

	return 0;
}
#endif