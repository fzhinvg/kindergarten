//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <type_traits>

class Base
{
private:
	int _value = 123;

public:
	// 显式构造函数
	explicit Base(int value) : _value(value)
	{}

	// 默认构造函数
	Base() = default;
//	Base() = delete;

	Base(const Base &other) : _value(other._value)
	{
		std::cout << "Copy constructor called" << std::endl;
	}

//	Base &operator=(const Base &other)=delete;
	Base &operator=(const Base &other)
	{
		std::cout << "Copy assignment operator called" << std::endl;
		if (this != &other)
		{
			_value = other._value;
		}
		return *this;
	}

	Base(Base &&other) noexcept: _value(other._value)
	{
		std::cout << "Move constructor called" << std::endl;
		other._value = 0; // 清理源对象
	}

	Base &operator=(Base &&other) noexcept
	{
		std::cout << "Move assignment operator called" << std::endl;
		if (this != &other)
		{
			_value = other._value;
			other._value = 0; // 清理源对象
		}
		return *this;
	}

	~Base() = default; // 默认的析构器实际上没有析构的过程，只有释放内存的过程，因为它是不必要的(关于为什么没有反汇编)

	void showValue() const
	{
		std::cout << "Value: " << _value << std::endl;
	}
};

inline bool isOdd(const int &i)
{
	return i & 0x1;
}

inline bool isOdd(double) = delete;


int main()
{
	Base b0{1};
	auto b1 = b0;
	auto b2 = std::move(b0);
//	auto b2 = static_cast<Base &&>(b0);
	b1 = b2;
	b1 = std::move(b2);
	std::cout << std::boolalpha << std::endl;
	std::cout << std::is_trivially_copyable<Base>::value << std::endl;
	std::cout << std::is_trivially_constructible<Base>::value << std::endl;
	std::cout << std::is_trivially_copy_assignable<Base>::value << std::endl;
	std::cout << std::is_trivially_move_assignable<Base>::value << std::endl;
	std::cout << std::is_destructible<Base>::value << std::endl; // true 说明是编译器自己生成的
	std::cout << std::is_constructible<Base>::value << std::endl;

	std::cout << std::endl;

	std::cout << ::isOdd(0) << std::endl;
	std::cout << isOdd(1) << std::endl;
	std::cout << isOdd(2) << std::endl;
	std::cout << isOdd(3) << std::endl;
	std::cout << isOdd(4) << std::endl;

	std::cout << sizeof(double) << std::endl;
	std::cout << sizeof(long double) << std::endl;


	return 0;
}