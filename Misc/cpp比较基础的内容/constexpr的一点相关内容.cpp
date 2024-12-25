//
// Created by fzhinvg on 2024/12/25.
//
#include <iostream>

constexpr const char *youAndMe = "Just for a second, Let me hold you tight";
const char *youAndMe_ = "Just for a second, I will hold my breath";
// 这两句的区别就在于constexpr就算使用O0未被使用也不会被编译,至少我的反汇编里没有

// 鼠鼠流完泪,就可以进入梦乡寻求慰藉
// 真正的勇士不会期盼任何抚慰,很可惜我不是

constexpr int factorial(int n) noexcept // 推荐加入noexcept,因为要在编译期使用,所以一般是不能抛出异常的
{
	return (n <= 1) ? 1 : (n * factorial(n - 1));
}

constexpr int square(int x) noexcept
{
	return x * x;
}

constexpr int arraySize(int n) noexcept
{
	return n * 2;
}

constexpr bool isPrime(int num)
{
	if (num <= 1) return false;
	for (int i = 2; i * i <= num; ++i)
	{
		if (num % i == 0) return false;
	}
	return true;
}

class Point
{
public:
	constexpr Point(double x, double y) : _x(x), _y(y)
	{}

	[[nodiscard]] constexpr double x() const
	{ return _x; }

	[[nodiscard]] constexpr double y() const
	{ return _y; }

private:
	double _x, _y;
};

int main()
{
	int fact1 = factorial(1); // 这一句会在运行时进入函数栈帧,下面两句则不会
	const int fact2 = factorial(2);
	constexpr int fact3 = factorial(3);

	// 同样使用断点调试即可观察到是否进入函数栈
	constexpr int compile_time_value = square(5);  // 编译时计算
	const int compile_time_value_ = square(6);  // 编译时计算
	int runtime_value = square(7);  // 运行时计算

	int myArray[arraySize(5)];  // 编译时计算数组大小为10

	constexpr int number = 17;
//	constexpr int number = 14;
	static_assert(isPrime(number), "Number must be prime");

	constexpr Point point(1.0, 2.0);
	/*
	 .L__const.main.point:
	.quad	0x3ff0000000000000
	.quad	0x4000000000000000
	 */
	static_assert(point.x() == 1.0, "X coordinate is incorrect");
	static_assert(point.y() == 2.0, "Y coordinate is incorrect"); // 相当于一个声明了一个静态存储区的的类对象

	/*
	 * constexpr对象 在编译期间已经确定并刻印在二进制文件中
	 * 而全局静态对象在程序运行时构造，并在程序结束时析构
	 * */

	return 0;
}
