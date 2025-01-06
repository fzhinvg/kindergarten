//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <iomanip>
#include <cstdint>

class Base
{
public:
	virtual void func1()
	{
		std::cout << "Base::func1()" << std::endl;
	}

	virtual void func2()
	{
		std::cout << "Base::func2()" << std::endl;
	}

	int i;
} base;

//class Derived : public Base
//{
//public:
//	void func1() override
//	{
//		std::cout << "Derived::func1()" << std::endl;
//	}
//
//	void func2() override
//	{
//		std::cout << "Derived::func2()" << std::endl;
//	}
//};

//void printVTable(Base *b)
//{
//	uintptr_t **vptr = reinterpret_cast<uintptr_t **>(b);
//	std::cout << "vptr: " << std::hex << reinterpret_cast<uintptr_t>(vptr) << std::endl;
//	std::cout << "vtable: " << std::hex << reinterpret_cast<uintptr_t>(*vptr) << std::endl;
//
//	for (int i = 0; i < 5; ++i)
//	{
//		std::cout << "vtable[" << i << "]: " << std::hex << (*vptr)[i] << std::endl;
//	}
//}

int get_int(const int &i)
{
	return i;
}

int main()
{
	auto bp = &base;
//	printVTable(bp);
	uintptr_t **vptr = reinterpret_cast<uintptr_t **>(bp);// get vptr
	uintptr_t *vtable = *vptr;// vtable == *vptr vptr解一次指针就是vtable
	auto vfunc_addr = (*vptr)[0];
	auto vfunc_addr2 = vtable[1];// 上下等价
//	auto vfunc_addr2 = *(vtable + 1);
	std::cout << std::hex << "vfunc addr " << vfunc_addr << std::endl;
	std::cout << "vfunc addr2 " << vfunc_addr2 << std::endl;

	using vfunc_t = void (*)();// virtual func_t
	vfunc_t vfunc = reinterpret_cast<vfunc_t>(vfunc_addr);
	vfunc();

	vfunc_t vfunc2 = reinterpret_cast<vfunc_t>(vfunc_addr2);
	vfunc2();

	return 0;
}
