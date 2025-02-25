//
// Created by fzhinvg on 2025/2/26.
//
#include <thread>
#include <print>
#include <array>
#include <future>
#include <chrono>
#include <exception>
#include <iostream>

#define enable_thread_basic_demo false
#define enable_promise_and_future_demo false
#define enable_wait_demo false

#pragma region thread_basic
#if enable_thread_basic_demo
void taskFunc(int a, int b, int &result)
{
	result = a + b;
}

void helloFunc(int idx)
{
	std::print("hello from thread {},id:{}\n", idx, std::this_thread::get_id());
}

int main()
{
	int result = 0;
	std::print("使用std::ref传递引用,作用类似于C#中的out: {:<2}", result);
	std::thread task{taskFunc, 60, 9, std::ref(result)}; // 有趣的是,thread会默认创建参数的拷贝用于传递,需要使用ref来表示引用传递
	if (task.joinable())
		task.join();
	std::print("结果: {:<2} \n", result);

	constexpr int thread_limit = 10;
	std::array<std::thread, thread_limit> threads{};
	for (auto &thread: threads)
	{
		static int idx = 0;
		threads[idx] = std::thread(helloFunc, idx); // thread对象只能移动构造和移动赋值,因为不能存在两个一模一样的线程
//		threads[idx] = std::move(std::thread(helloFunc, idx)); // 效果一样
	}
	for (auto &thread:threads)
	{
		if (thread.joinable())
			thread.join(); // 这个循环会让主线程阻塞,循环执行时间取决于这一堆线程中运行时长最长的那个
	}


	return 0;
}
#endif
#pragma endregion

#pragma region promise_and_future
#if enable_promise_and_future_demo

// 一个类比,这两个对象给线程通信提供了一种简单的解决方案,它们是一对一的
// promise -> setter
// future -> getter

void producer(std::promise<int> &&p)
{
	// 传递数据
//	std::this_thread::sleep_for(std::chrono::seconds(2));
//	p.set_value(0x4996'02D2);
//	std::println("producer set value");
//	std::this_thread::sleep_for(std::chrono::seconds(2));
	// 传递 exception
	try
	{
		std::println("producer will throw an exception");
		throw std::runtime_error{"this is an exception: runtime_error"};
	} catch (...)
	{
		p.set_exception(std::current_exception());
	}
	std::this_thread::sleep_for(std::chrono::seconds(2));

	std::println("producer executed done");
}

void consumer(std::future<int> &&f)
{
	// 使用数据, future 只能get一次
//	std::println("consumer:{}", f.get());
//	std::this_thread::sleep_for(std::chrono::seconds(3));
	// std::shared_future 可以多次调用 get() 获取结果,但它对应的 std::promise 依然只能设置一次值,多次设置会抛出 std::future_error 异常,其设计的核心职责是单次结果传递
	// 因为它主要是给多个消费者线程实现共享使用的
	// std::future::share() 方法将 std::future 转换为可复制的状态的 std::shared_future;

	// 如果需要多次传递值,可以使用消息队列和锁,stl的容器基本上是线程不安全的,或者使用std::async
	try
	{
		f.get();
	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::seconds(3));

	std::println("consumer executed done");
}

void producer_ref(std::promise<int> &p)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	p.set_value(0x4996'02D2);
	std::this_thread::sleep_for(std::chrono::seconds(3));
}

// 这两个方法不是这份代码的重点
#if use_message_queue
std::queue<int> msg_queue;
std::mutex mtx;
std::condition_variable cv;

void producer()
{
	for (int i = 0; i < 10; ++i)
	{
		std::lock_guard<std::mutex> lock(mtx); // 加锁
		msg_queue.push(i);                     // 写操作受锁保护
		cv.notify_one();
	}
}

void consumer()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, []
		{ return !msg_queue.empty(); }); // 条件变量等待
		int value = msg_queue.front();
		msg_queue.pop();                        // 读操作受锁保护
		std::cout << "Received: " << value << std::endl;
	}
}
#endif
#if use_async
#include <functional>

void async_task(std::function<void(int)> callback)
{
	for (int i = 0; i < 10; ++i)
	{
		callback(i); // 多次回调传值
	}
}

int main()
{
	auto future = std::async(std::launch::async, [&]
	{
		async_task([](int value)
				   {
					   std::cout << "Received: " << value << std::endl;
				   });
	});
	future.wait();
}
#endif

int main()
{
	{
		std::promise<int> promise;
		std::future<int> future = promise.get_future();
		// promise 和 future 不可拷贝,只能移动

		// 必须用右值传递promise
		std::thread producer_task{producer, std::move(promise)};
		std::thread consumer_task{consumer, std::move(future)};

		if (producer_task.joinable())
			producer_task.join();
		if (consumer_task.joinable())
			consumer_task.join();

	} // 此行用于演示生命周期的释放位置
	std::println("before main ret"); // 此行用于演示生命周期的释放位置

	return 0;
}
// 关于promise和future的生命周期
#define lifecycle false
#if lifecycle
int main() {
	// 1. 创建 promise 和 future,共享状态被初始化
	std::promise<int> p;
	std::future<int> f = p.get_future();

	// 2. 将 promise 移动到生产者线程
	std::thread producer([&p] {
		p.set_value(42); // 设置值,共享状态标记为就绪
		// p 在线程结束时销毁,但共享状态仍存活(被 f 引用)
	});

	// 3. 将 future 移动到消费者线程
	std::thread consumer([&f] {
		int val = f.get(); // 获取值,共享状态完成
		// f 变为无效(valid() == false)
	});

	producer.join();
	consumer.join();

	// 4. 所有线程结束，共享状态在此处释放(无引用)
	return 0;
}
#endif

#endif
#pragma endregion

#pragma region wait_demo
#if enable_wait_demo

#include <atomic>
#include <vector>

// 模拟一个耗时任务
int long_running_task(int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
	return 42;
}

int main()
{
	// ==============================
	// 场景1:基础等待 阻塞式
	// ==============================
	{
		std::cout << "\n===== basic =====" << std::endl;
		std::future<int> fut = std::async(std::launch::async, []
		{
			return long_running_task(2); // 耗时2秒的任务
		});

		std::cout << "main thread wait for result..." << std::endl;
		fut.wait(); // 阻塞直到任务完成
		std::cout << "task done: " << fut.get() << std::endl;
	}

	// ==============================
	// 场景2:超时等待 非阻塞式
	// ==============================
	{
		std::cout << "\n===== Time out =====" << std::endl;
		std::future<int> fut = std::async(std::launch::async, []
		{
			return long_running_task(3); // 耗时3秒的任务
		});

		// 循环检查状态
		while (true)
		{
			auto status = fut.wait_for(std::chrono::seconds(1)); // 每次等待1秒
			if (status == std::future_status::ready)
			{
				std::cout << "task done: " << fut.get() << std::endl;
				break;
			} else if (status == std::future_status::timeout)
			{
				std::cout << "time out,keep waiting..." << std::endl;
			}
		}
	}

	// ==============================
	// 场景3:多线程等待共享结果
	// ==============================
	{
		std::cout << "\n===== Multi-threading =====" << std::endl;
		std::promise<int> prom;
		std::shared_future<int> shared_fut = prom.get_future().share();

		// 启动3个消费者线程
		std::vector<std::thread> consumers;
		consumers.reserve(3);
		for (int i = 0; i < 3; ++i)
		{
			consumers.emplace_back([i, shared_fut]
								   {
									   std::println("consumer{}waiting...",i);
									   shared_fut.wait(); // 所有线程在此同步
									   std::println("consumer{}get...{}",i,shared_fut.get());
								   });
		}

		// 生产者线程设置结果
		std::thread producer([&prom]
							 {
								 std::this_thread::sleep_for(std::chrono::seconds(2));
								 prom.set_value(100);
								 std::println("produce a number");
							 });

		producer.join();
		for (auto &t: consumers) t.join();
	}

	// ==============================
	// 场景4:异常处理
	// ==============================
	{
		std::cout << "\n===== Exception =====" << std::endl;
		std::future<void> fut = std::async(std::launch::async, []
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			throw std::runtime_error("task failure");
		});

		try
		{
			fut.wait();
			fut.get(); // 会重新抛出异常
		} catch (const std::exception &e)
		{
			std::cout << "caught an exception: " << e.what() << std::endl;
		}
	}

	return 0;
}

#endif
#pragma endregion
