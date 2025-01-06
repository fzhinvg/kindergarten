//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>

class Base
{
public:
	Base()
	{
		init();
	}

	virtual void init()
	{
		std::cout << "Base init" << std::endl;
	}

	virtual ~Base()
	{}
};

class Derived : public Base
{
public:
	void init() override
	{
		std::cout << "Derived init" << std::endl;
	}
};

int main()
{
	Derived d;
	return 0;
}
//当创建 Derived 对象时，Base 的构造函数会被调用。在 Base 的构造函数中调用了虚函数 init，由于此时 Derived 对象还未完全构造，因此调用的是 Base 的 init 实现，而不是 Derived 的 init 实现。

//更新：
#if 0
#include <iostream>
#include <new>

class Base
{
public:
	Base()
	{
		init();// BP
	}

	virtual void init()
	{
		std::cout << "Base init" << std::endl;
	}

	virtual ~Base()
	{
		std::cout<<std::endl;// BP
	};
};

class Derived : public Base
{
public:
	Derived()
	{
		init();// BP
	};
	void init() override
	{
		std::cout << "Derived init" << std::endl;
	}
	~Derived()
	{
		std::cout<<std::endl;// BP
	}
};

int main()
{
	void* memory= operator new(sizeof(Derived));

	auto derived=new(memory)Derived();

	delete derived;



	return 0;
}
//当创建 Derived 对象时，Base 的构造函数会被调用。
// 在 Base 的构造函数中调用了虚函数 init，由于此时 Derived 对象还未完全构造，
// 因此调用的是 Base 的 init 实现，而不是 Derived 的 init 实现。//ver 1
// ver2: 在注释BP的行打上断点可以明显看出来构造的过程中vptr的变向，
// 说明构造器在使用虚函数的时候vtable已经是明确指向当前类型了
// 但是还是不建议在构造器中使用虚函数，因为这个玩具代码的示例结构太简单，所以很容易看出来构造与析构的过程
// 又我试验了一下，发现vptr的改写时机对于构造器与析构器不同，
// 构造器是在内部语句使用之前就被改写，析构器是在内部语句运行完成之后才被改写
#endif