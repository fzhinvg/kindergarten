//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <cstdint>

class Base
{
public:
	virtual void print() const
	{
		std::cout << "This is the base class implementation." << std::endl;
	}
};

class Derived : public Base
{
public:
	void print() const override
	{
		std::cout << "This is the derived class implementation." << std::endl;
	}
	void callBasePrint()
	{
		Base::print();// 写这行是为了不写这句
	}
};

void callBasePrintViaPointer(Derived derived)
{
	Base base = static_cast<Base>(derived);// 使用对象切片剥离继承部分
	base.print();// 这样不就得了
//	Base *basePtr = dynamic_cast<Base *>(&derived);// 动态转换无法剥离,实际上是同一个对象,只是指针的解读不一样
//
//	auto vptr = reinterpret_cast<uintptr_t **>(&base);
//	uintptr_t *vtable = *vptr;
//
//	using vfunc_t = void (*)();
//	vfunc_t basePrint = reinterpret_cast<vfunc_t>(vtable[0]);
//
//	// 调用基类的 print() 方法
//	basePrint();
}

void callBasePrintViaPointer_alter(Derived& derived) {
	// 使用对象切片剥离继承部分
	Base base = static_cast<Base>(derived);
	base.print(); // 调用基类的 print 方法

	// 动态转换
	Base* basePtr = dynamic_cast<Base*>(&derived);
	basePtr->print(); // 调用派生类的 print 方法
}

int main()
{
	Derived derived;
//	derived.print();              // 调用派生类的 print() 实现
//	callBasePrintViaPointer(derived); // 显式调用基类的 print() 实现

//	auto derivedPtr = new Derived();
//	Base *basePtr = static_cast<Base *>(derivedPtr); // 安全的 upcasting
//	delete derivedPtr;
	callBasePrintViaPointer_alter(derived);

	return 0;
}
