//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;
bool dataProduced = false;

void producer()
{
	std::unique_lock<std::mutex> lock(mtx);
	std::cout << "Producer has locked the mutex and is producing data..." << std::endl;
	dataProduced = true;  // 模拟生产数据
	ready = true;
	cv.notify_one();  // 通知消费者数据已准备好
	std::cout << "Producer has notified consumer" << std::endl;
	// lock 会在作用域结束时自动解锁
}

void consumer()
{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, []
	{ return ready; });  // 等待通知
	std::cout << "Consumer has adopted the lock and is consuming data..." << std::endl;
	if (dataProduced)
	{
		// 处理数据
		dataProduced = false;
	}
	// lock 会在作用域结束时自动解锁
}

int main()
{
	std::thread t1(producer);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 确保生产者先获取锁
	std::thread t2(consumer);

	t1.join();
	t2.join();

	return 0;
}

#if ver2
//ver2
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> dataQueue;
bool done = false;

void producer()
{
	for (int i = 0; i < 10; ++i)
	{
		{
			std::unique_lock<std::mutex> lock(mtx);
			dataQueue.push(i);
			std::cout << "Produced: " << i << std::endl;
		}
		cv.notify_one(); // 通知消费者
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟生产时间
	}
	{
		std::unique_lock<std::mutex> lock(mtx);
		done = true;
	}
	cv.notify_all(); // 通知所有消费者生产已完成
}

void consumer()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, []
		{ return !dataQueue.empty() || done; });
		if (!dataQueue.empty())
		{
			int data = dataQueue.front();
			dataQueue.pop();
			std::cout << std::this_thread::get_id() << "Consumed: " << data << std::endl;
		} else if (done)
		{
			break;
		}
	}
}

int main()
{
	std::thread prod(producer);
	std::thread cons1(consumer);
	std::thread cons2(consumer);

	prod.join();
	cons1.join();
	cons2.join();

	return 0;
}
#endif

#if ver3
//ver3
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <atomic>

std::mutex mtx;
std::condition_variable cv_producer, cv_consumer;
std::vector<int> buffer1, buffer2;
std::atomic<bool> useBuffer1{true}; // 使用原子布尔值来避免锁竞争
bool done = false;
const int BUFFER_SIZE = 5; // 缓冲区大小

void producer()
{
	for (int i = 0; i < 20; ++i)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv_producer.wait(lock, []
		{
			return (useBuffer1.load() ? buffer1.size() < BUFFER_SIZE : buffer2.size() < BUFFER_SIZE);
		});

		if (useBuffer1.load())
		{
			buffer1.push_back(i);
		} else
		{
			buffer2.push_back(i);
		}
		std::cout << "Produced: " << i << std::endl;

		if ((useBuffer1.load() && buffer1.size() == BUFFER_SIZE) ||
			(!useBuffer1.load() && buffer2.size() == BUFFER_SIZE))
		{
			useBuffer1.store(!useBuffer1.load()); // 切换缓冲区
			cv_consumer.notify_all(); // 通知所有消费者
		}
	}

	{
		std::unique_lock<std::mutex> lock(mtx);
		done = true;
		cv_consumer.notify_all(); // 通知所有消费者生产已完成
	}
}

void consumer()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv_consumer.wait(lock, []
		{
			return (!useBuffer1.load() ? !buffer1.empty() : !buffer2.empty()) || done;
		});

		if ((!useBuffer1.load() && !buffer1.empty()) || (useBuffer1.load() && !buffer2.empty()))
		{
			std::vector<int> &buffer = useBuffer1.load() ? buffer2 : buffer1;
			while (!buffer.empty())
			{
				int data = buffer.back();
				buffer.pop_back();
				lock.unlock();
				std::cout << std::this_thread::get_id() << " Consumed: " << data << std::endl;
				lock.lock();
			}
			useBuffer1.store(!useBuffer1.load()); // 切换缓冲区
			cv_producer.notify_all(); // 通知所有生产者
		} else if (done)
		{
			break;
		}
	}
}

int main()
{
	std::thread prod(producer);
	std::thread cons1(consumer);
	std::thread cons2(consumer);

	prod.join();
	cons1.join();
	cons2.join();

	return 0;
}
#endif

