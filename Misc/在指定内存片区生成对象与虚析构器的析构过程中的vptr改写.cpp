//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>

class Base
{
public:
	bool destroyed = false;

	virtual void virtualFunction() const
	{
		std::cout << "Base virtual function implementation." << std::endl;
	}

	virtual ~Base()
	{//改写vptr指向base::vtable来访问基类虚构器，当析构函数是虚函数时；很难相信这是半个括号编译出来的
		/*
		 *	lea	rdx, vtable for Base[rip+16]
		 *	mov	rax, QWORD PTR 16[rbp]
		 *	mov	QWORD PTR [rax], rdx
		 */
		destroyed = true;
		std::cout << "Base destructor called. Object marked as destroyed." << std::endl;
	}
};

class Derived : public Base
{
public:
	int x;

	Derived(int val) : x(val)
	{
		std::cout << "Derived constructor called with value: " << x << std::endl;
	}

	void virtualFunction() const override
	{
		if (destroyed)
		{
			std::cout << "Object is destroyed, virtual function cannot proceed." << std::endl;
			// 我有一种这一句永远不会被触发的感觉
		} else
		{
			std::cout << "Derived virtual function implementation with value: " << x << std::endl;
		}
	}

	~Derived() override
	{
		std::cout << "Derived destructor called. Object marked as destroyed." << std::endl;
	}
};

int main()
{
	void *memory = operator new(sizeof(Derived));
	Derived *obj = new(memory) Derived(0x1BADB002);

	obj->virtualFunction();

	obj->~Derived();
//	delete obj;// 写着行是为了说明delete和析构器的区别，delete完成了这里的析构和释放内存两个部分
	obj->virtualFunction();// 尝试在对象销毁后访问虚函数，这将调用基类的虚函数实现

	operator delete(memory);// 重复释放还会发生 Signal: SIGTRAP (Trace/breakpoint trap)
#if 0
	// 这是复用
	void* memory = operator new(sizeof(Derived));
    Derived* obj = new (memory) Derived(0x1BADB002);

    obj->virtualFunction();

    // 销毁当前对象
    obj->~Derived();

    // 释放内存
    operator delete(memory);

    // 重新分配内存并构造新对象
    memory = operator new(sizeof(Derived));
    obj = new (memory) Derived(0xCAFEBABE);

    obj->virtualFunction();

    // 记得最后再次销毁对象和释放内存
    obj->~Derived();
    operator delete(memory);

    return 0;
#endif
	return 0;
}

