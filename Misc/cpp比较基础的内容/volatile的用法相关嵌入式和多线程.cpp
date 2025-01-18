//
// Created by fzhinvg on 2025/1/18.
//
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

// 每次访问这个变量时都要直接从内存中读取它的值,背后的话其实是"每次都从ram中读取,不进行cache优化"
// 某种意义上是限制编译器进行自动优化
// 假设这是一个指向传感器数据寄存器的指针(模拟环境)
volatile int sensorData = 0;

void simulateSensorDataUpdate()
{
	for (int i = 0; i < 10; ++i)
	{
		sensorData = i;  // 更新传感器数据
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 模拟数据更新间隔
	}
}

int main()
{
	// 启动一个线程模拟传感器数据更新
	std::thread sensorThread(simulateSensorDataUpdate);

//	std::this_thread::sleep_for(std::chrono::seconds(1));
	// 主线程读取传感器数据
	for (int i = 0; i < 10; ++i)
	{
		int data = sensorData;
		std::cout << "Sensor data: " << data << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 模拟读取间隔
	}

	// 等待传感器更新线程结束
	sensorThread.join();

	return 0;
}

// volatile 适用于标志变量,这些变量在一个线程中被设置或修改,而在其他线程中被读取.例如,停止线程的标志
// 多线程中的应用场合:
#if 0
volatile bool stopFlag = false;  // 使用 volatile 声明的标志变量

void worker() {
	while (!stopFlag) {
		// 执行某些任务
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << "Working..." << std::endl;
	}
	std::cout << "Worker thread stopped." << std::endl;
}

int main() {
	std::thread t(worker);

	// 让工作线程运行一段时间
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// 修改标志变量，通知工作线程停止
	stopFlag = true;

	t.join();
	std::cout << "Main thread stopped." << std::endl;
	return 0;
}
#endif
