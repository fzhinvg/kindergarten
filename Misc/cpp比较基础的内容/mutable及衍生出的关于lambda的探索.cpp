//
// Created by fzhinvg on 2025/1/15.
//
#include <iostream>

class MyClass
{
public:
	void modify() const
	{
		++counter;
	}

	int getCounter() const
	{
		return counter;
	}

private:
	mutable int counter = 0; // 1.mutable 使其在 const 方法中可以被修改
};

int main()
{
	int x = 10;
	// 2.捕获列表中按值捕获的变量默认是不可修改,但可以通过 mutable 关键字允许修改
	auto lambda = [x]() mutable
	{
		x += 10;
		std::cout << "Modified x: " << x << std::endl;
	};

	lambda();  // 输出 20
	std::cout << "Original x: " << x << std::endl;  // 输出 10，因为 x 是按值捕获

	MyClass obj;
	obj.modify();
	std::cout << "Counter: " << obj.getCounter() << std::endl;  // 输出 1


	auto la = [x]
	{ return; }; // auto被推断为 void() const

	void(*la_)(int)=[](int){return;}; // 这种写法不支持捕获

	return 0;
}
// 由此拓展的关于lambda的探索
// 我们察觉到两点,一是当以值捕获的时候,其默认为int() const
// 二是lambda本质上是一个对象,捕获内容就是他的内存布局,lambda内部语句是它的operator()
#if 0

#include <cstdio>

int main()
{
	int x = 1;
	auto lambda = [x]
	{ return x; };
	return 0;
}

// ↓

#include <cstdio>

int main()
{
	int x = 1;

	class __lambda_6_15
	{
	public:
		inline /*constexpr */ int operator()() const
		{
			return x;
		}

	private:
		int x;

	public:
		__lambda_6_15(int &_x)
				: x{_x}
		{}

	};

	__lambda_6_15 lambda = __lambda_6_15{x};
	return 0;
}

// 上述由https://cppinsights.io/生成 之前一直不知道这个网站的具体功能
#endif

