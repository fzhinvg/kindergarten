//
// Created by fzhinvg on 2025/2/27.
//
#include <string>
#include <format> // 用于生成字符串
#include <print> // 用于格式化输出,体验比使用cout强点
#include <iostream>
#include <source_location>
#include <iomanip>
#include <sstream>

#pragma region custom_type_format
struct Point
{
	int x;
	int y;
};

// 自定义类型的格式化
template<>
struct std::formatter<Point>
{
	// 告诉std::format如何解析格式字符串中的额外参数 例如 ":.2f"
	// 若不需要,直接返回 ctx.begin()
	constexpr auto parse(std::format_parse_context &ctx) noexcept
	{
		// std::format_context,一个输出上下文对象,封装了输出缓冲区和相关状态
		// 通过 ctx.out() 获取输出迭代器
		return ctx.begin();
	}

	// 核心函数,定义如何将对象转换为字符串
	auto format(const Point &p, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "({},{})", p.x + 123, p.y + 246);
	}
	// 这两个函数并不应该被声明为static,这样会导致它们失去this指针,违反接口要求
	// noexcept,不涉及动态内存就比较合适,比如这里的parse()比较简单
	// 但是涉及到调用别的函数和动态分配内存,如果抛出异常会直接调用std::terminate,而不能正常处理异常
};

#pragma endregion

#pragma region log_demo

[[nodiscard]]
std::string timestamp_ms()
{
	// 获取当前时间点
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// 提取毫秒部分
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	// 线程安全的本地时间转换(跨平台)
	struct tm tm_buf{};
#if defined(_WIN32)
	localtime_s(&tm_buf, &in_time_t);      // Windows 线程安全版本
#else
	localtime_r(&in_time_t, &tm_buf);      // Linux/macOS 线程安全版本
#endif

	std::stringstream ss;
	ss << "[" << std::put_time(&tm_buf, "%H:%M:%S")  // %Y-%m-%d 年月日format
	   << "." << std::setfill('0') << std::setw(3) << ms.count()  // 毫秒位 .3f
	   << "]";

	return ss.str();
}

void log(const std::string &message,
		 const std::source_location &loc = std::source_location::current())
{
	std::print("{} [{}:{}] {}\n",
			   timestamp_ms(), loc.file_name(), loc.line(), message);
}

#pragma endregion

int main()
{
#pragma region format_demo
	std::string mani{std::format({"紅葉の锦 神のまにまに-{}"}, "麹町養蚕館")};
	std::cout << mani << std::endl;

	std::string cs = std::format("{1} {0} -{2}", "Schatze", "Clouds", "LV.4");
	std::cout << cs << std::endl;

	double pi = 3.1415926535;
	std::string pi_0 = std::format("pi={:>8.5f}", pi);
	std::string pi_1 = std::format("{:*^11}", "cpp");
	std::cout << pi_0 << "\n" << pi_1 << std::endl;
#pragma endregion

#pragma region print_demo
	// std::print 基于 std::format 用法兼容
	std::print("Hello {}\n", "World!");
#pragma endregion

#pragma region custom_type_format
	Point p{123, 246};
	std::print("Point:{}\n", p);

#pragma endregion

#pragma region log_demo
	log(std::format("Pure Emotion {}", p));
#pragma endregion
	return 0;
}