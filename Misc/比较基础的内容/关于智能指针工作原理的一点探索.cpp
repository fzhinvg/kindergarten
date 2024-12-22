//
// Created by fzhinvg on 2024/12/23.
//
#include <algorithm>
#include <iostream>
#include <string>
#include <exception>
#include <atomic>
#include <mutex>
//#include <memory>

template<typename T>
class u_ptr // unique_ptr
{
private:
	T *_ptr;
public:
	explicit u_ptr(T *ptr = nullptr) : _ptr(ptr)
	{
		std::cout << __func__ << std::endl;
	};

	~u_ptr()
	{
		std::cout << __func__ << std::endl;
		delete _ptr;
	}

	// unique_ptr 独占不允许拷贝,禁用相关拷贝操作
	u_ptr(const u_ptr &) = delete;

	u_ptr &operator=(const u_ptr &) = delete;

	u_ptr(u_ptr &&other) noexcept: _ptr(other._ptr)
	{
		std::cout << __func__ << " move constructor" << std::endl;
		other._ptr = nullptr;
	}

	u_ptr &operator=(u_ptr &&other) noexcept
	{
		std::cout << __func__ << " move" << std::endl;
		if (this != &other) // 自我过滤
		{
			delete _ptr;
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}

	T &operator*() const
	{
		std::cout << __func__ << std::endl;
		return *_ptr;
	}

	T *operator->() const
	{
		std::cout << __func__ << std::endl;
		return _ptr;
	}

	[[nodiscard]] T *release()
	{
		std::cout << __func__ << std::endl;
		T *temp = _ptr;
		_ptr = nullptr;
		return temp;
	}

	void reset(T *ptr = nullptr)
	{
		std::cout << __func__ << std::endl;
		if (ptr != _ptr)
		{
			delete _ptr;
			_ptr = ptr;
		}
//		std::swap(_ptr, ptr);
//		delete ptr;
	}

};

#if sharedOnlyVersion
template<typename T>
class s_ptr // shared_ptr
{
private:
	T *_ptr;
	std::atomic<size_t> *_ref_count;
	std::mutex mtx;

	void release()
	{
		std::cout << __func__ << std::endl;
		std::lock_guard<std::mutex> lock(mtx); // thread safe
		if (_ref_count && --(*_ref_count) == 0) // 计数存在且==1,此时属于独占所以允许被释放
		{
			delete _ptr;
			delete _ref_count;
		}
	}

public:
	// 大括号初始化{}防止窄化转换(narrowing conversion)
	// 圆括号初始化 () 是传统的构造函数语法,用于调用构造函数来初始化对象
	// 使用{}时,编译器会阻止窄化转换,旨在防止潜在的类型转换错误和数据丢失
	explicit s_ptr(T *ptr = nullptr) : _ptr(ptr), _ref_count(new std::atomic<size_t>{1})
	{
		std::cout << __func__ << std::endl;
	}

	~s_ptr()
	{
		release();
	}

	s_ptr(const s_ptr &other) : _ptr(other._ptr), _ref_count(other._ref_count)
	{
		std::cout << __func__ << " copy constructor" << std::endl;
		++(*_ref_count);
	}

	s_ptr(s_ptr &&other) noexcept: _ptr(other._ptr), _ref_count(other._ref_count)
	{
		std::cout << __func__ << " move constructor" << std::endl;
		other._ptr = nullptr;
		other._ref_count = nullptr;
	}

	s_ptr &operator=(const s_ptr &other)
	{
		std::cout << __func__ << std::endl;

		if (this != &other) // 自我过滤
		{
			release();
			_ptr = other._ptr;
			_ref_count = other._ref_count;
			++(*_ref_count);
		}
		return *this;
	}

	s_ptr &operator=(s_ptr &&other) noexcept
	{
		std::cout << __func__ << " move" << std::endl;
		if (this != &other) // // 自我过滤
		{
			release();
			_ptr = other._ptr;
			_ref_count = other._ref_count;
			other._ptr = nullptr;
			other._ref_count = nullptr;
		}
		return *this;
	}

	T &operator*() const
	{
		return *_ptr;
	}

	T *operator->() const
	{
		return _ptr;
	}

	[[nodiscard]] size_t use_count() const // 也不会调用任何可能修改类成员的非 const 方法
	{
		// 总是认为只是不让修改成员变量,其实他有俩作用哈哈
		// 其实也并非一定需要,写了给编译器检查用,因为这里结构简单所以显得没啥必要
		// 但是这是一种好习惯,同样增加了可读性,一眼就能看出来这是一个不会修改对象的method
		return *_ref_count;
	}

	void reset(T *ptr = nullptr)
	{
		release();
		_ptr = ptr; // 使用新对象承接,同时重置计数器
		_ref_count = new std::atomic<size_t>{1};
	}

};
#endif

template<typename T>
class w_ptr;

template<typename T>
class s_ptr // shared_ptr&weak_ptr协作最小演示可用版本
{
private:
	T *_ptr;
	std::atomic<size_t> *_ref_count;
	std::atomic<size_t> *_weak_count; // 新增计数器
	std::mutex mtx;

	void release()
	{
		std::cout << "s_ptr::" << __func__ << std::endl;
		std::lock_guard<std::mutex> lock(mtx); // thread safe
		if (_ref_count && --(*_ref_count) == 0) // 计数存在且==1,此时属于独占所以允许被释放
		{
			delete _ptr;
			if (_weak_count && *_weak_count == 0) // 增加_weak_count释放
			{
				delete _ref_count;
				delete _weak_count;
			}
		}
	}

public:
	// 大括号初始化{}防止窄化转换(narrowing conversion)
	// 圆括号初始化 () 是传统的构造函数语法,用于调用构造函数来初始化对象
	// 使用{}时,编译器会阻止窄化转换,旨在防止潜在的类型转换错误和数据丢失
	explicit s_ptr(T *ptr = nullptr) : _ptr(ptr),
									   _ref_count(new std::atomic<size_t>{1}),
									   _weak_count(new std::atomic<size_t>{0})
	{
		std::cout << __func__ << std::endl;
	}

	~s_ptr()
	{
		release();
	}

	s_ptr(const s_ptr &other) : _ptr(other._ptr),
								_ref_count(other._ref_count),
								_weak_count(other._weak_count)
	{
		std::cout << __func__ << " copy constructor" << std::endl;
		++(*_ref_count);
	}

	s_ptr(s_ptr &&other) noexcept: _ptr(other._ptr),
								   _ref_count(other._ref_count),
								   _weak_count(other._weak_count)
	{
		std::cout << __func__ << " move constructor" << std::endl;
		other._ptr = nullptr;
		other._ref_count = nullptr;
		other._weak_count = nullptr;
	}

	s_ptr &operator=(const s_ptr &other)
	{
		std::cout << __func__ << std::endl;

		if (this != &other) // 过滤自我赋值
		{
			release();
			_ptr = other._ptr;
			_ref_count = other._ref_count;
			_weak_count = other._weak_count;
			++(*_ref_count);
		}
		return *this;
	}

	s_ptr &operator=(s_ptr &&other) noexcept
	{
		std::cout << __func__ << " move" << std::endl;
		if (this != &other)
		{
			release();
			_ptr = other._ptr;
			_ref_count = other._ref_count;
			_weak_count = other._weak_count;
			other._ptr = nullptr;
			other._ref_count = nullptr;
			other._weak_count = nullptr;
		}
		return *this;
	}

	T &operator*() const
	{
		return *_ptr;
	}

	T *operator->() const
	{
		return _ptr;
	}

	[[nodiscard]] size_t use_count() const // 也不会调用任何可能修改类成员的非 const 方法
	{
		// 总是认为只是不让修改成员变量,其实他有俩作用哈哈
		// 其实也并非一定需要,写了给编译器检查用,因为这里结构简单所以显得没啥必要
		// 但是这是一种好习惯,同样增加了可读性,一眼就能看出来这是一个不会修改对象的method
		return *_ref_count;
	}

	void reset(T *ptr = nullptr)
	{
		release();
		_ptr = ptr; // 使用新对象承接,同时重置计数器
		_ref_count = new std::atomic<size_t>{1};
		_weak_count = new std::atomic<size_t>{0};
	}

	friend class w_ptr<T>;
};

template<typename T>
class w_ptr // weak_ptr 我可能把这个家伙想的有点弱了,可一点都不像名字一样weak,天快亮了,我想,我应该明天再接着重构
{
private:
	T *_ptr;
	std::atomic<size_t> *_ref_count;
	std::atomic<size_t> *_weak_count;
	std::mutex mtx;

//	void release()
//	{
//		std::cout << __func__ << std::endl;
//		std::lock_guard lock(mtx);
//		if (_weak_count && --(*_weak_count) == 0
//			&& (!_ref_count || *_ref_count == 0))
//		{
//			delete _weak_count;
//			if (_ref_count && *_ref_count == 0)
//			{
//				delete _ref_count;
//			}
//		}
//	}

	// alter version
	void release()
	{
		std::cout << "w_ptr::" << __func__ << std::endl;
		std::lock_guard<std::mutex> lock(mtx);
		if (_weak_count && --(*_weak_count) == 0 && (!_ref_count || *_ref_count == 0))
		{
			delete _weak_count;
			_weak_count = nullptr;
			if (_ref_count && *_ref_count == 0)
			{
				delete _ref_count;
				_ref_count = nullptr;
			}
		}
	}


public:
	w_ptr() : _ptr(nullptr), _ref_count(nullptr), _weak_count(nullptr)
	{
		std::cout << __func__ << std::endl;
	}

	explicit w_ptr(const s_ptr<T> &sptr) : _ptr(sptr._ptr),
										   _ref_count(sptr._ref_count),
										   _weak_count(sptr._weak_count)
	{
		std::cout << __func__ << std::endl;
		if (_weak_count)
		{
			++(*_weak_count);
		}
	}

	w_ptr(const w_ptr<T> &other) : _ptr(other._ptr),
								   _ref_count(other._ref_count),
								   _weak_count(other._weak_count)
	{
		std::cout << __func__ << std::endl;
		if (_weak_count)
		{
			++(*_weak_count);
		}
	}

	w_ptr &operator=(const w_ptr<T> &other)
	{
		if (this != &other) // 自我过滤
		{
			release();
			_ptr = other._ptr;
			_ref_count = other._ref_count;
			_weak_count = other._weak_count;
			if (_weak_count)
			{
				++(*_weak_count);
			}
		}
		return *this;
	}

	s_ptr<T> lock() const
	{
		if (_ref_count && *_ref_count > 0)
		{
			return s_ptr<T>(*this);
		}
		return s_ptr<T>{};
	}

	~w_ptr()
	{
		release();
	}

	friend class s_ptr<T>;

	w_ptr &operator=(const s_ptr<T> &sptr) // sptr -> wptr 的转换
	{
		release();
		_ptr = sptr._ptr;
		_ref_count = sptr._ref_count;
		_weak_count = sptr._weak_count;
		if (_weak_count)
		{
			++(*_weak_count);
		}
		return *this;
	}

};

class MyClass
{
public:
	MyClass()
	{
		std::cout << "Constructor called" << std::endl;
	}

	~MyClass()
	{
		std::cout << "Destructor called" << std::endl;
	}
};

class Node
{
public:
	s_ptr<Node> prev;
//	s_ptr<Node> next; // 这样析构器应该是不会被触发,现在它总算是不会被触发了,这个该死的小怪物难调得逆天
	w_ptr<Node> next;

	~Node()
	{
		std::cout << __func__ << std::endl;
	}
};

#define unique true
#define shared true
#define weak true

int main()
{
	using std::string;
	std::cout << "=====================>unique : " << std::endl;
#if unique
	try
	{
		// 智能指针本身并不是动态对象，而是用来管理动态分配的对象
		// 栈上创建，管理堆上对象
		// 所以说我应该像局部变量一样使用它，而不是new出来的动态对象(这样做有点本末倒置)
		u_ptr<int> int_u_ptr{new int{123123}};
		auto other_ptr = static_cast<u_ptr<int> &&>(int_u_ptr); // 因为很酷
		std::cout << std::endl;
		throw std::runtime_error("u_ptr exception");
		// 不需要手动删除, other_ptr 离开作用域时会自动释放资源
		std::cout << *other_ptr << std::endl;
	}
	catch (std::runtime_error &e)
	{
		std::cout << "handle exception: " << e.what() << std::endl << std::endl;
	}

//	MyClass obj;
//	obj.~MyClass(); // 手动调用析构函数 double free maybe 没事不要手动调用析构器
#endif
	std::cout << "=====================>shared : " << std::endl;
#if shared
	{
		s_ptr<string> s_s_ptr{new string{"moor"}};
		std::cout << *s_s_ptr << std::endl;
		std::cout << s_s_ptr.use_count() << std::endl;
		{
			auto s_s_ptr2 = s_s_ptr;
			std::cout << s_s_ptr.use_count() << std::endl;
			std::cout << s_s_ptr2.use_count() << std::endl;

			auto s_s_ptr3 = std::move(s_s_ptr2);
			std::cout << s_s_ptr.use_count() << std::endl;
			std::cout << s_s_ptr3.use_count() << std::endl;
		}
		std::cout << s_s_ptr.use_count() << std::endl;
	}
	std::cout << std::endl;
#endif
	std::cout << "=====================>weak : " << std::endl;
#if weak

	{
//		s_ptr<Node> node1;
//		s_ptr<Node> node2; // 原本我这样图方便没有分配一个地址给node中的 _ptr 导致它为nullptr
//						   // 以及后续触发的一系列事件让this指针指向一个0x20 在release()中它无法获取到正确的锁的地址,导致segment fault
		s_ptr<Node> node1(new Node());
		s_ptr<Node> node2(new Node());

		node1->next = node2;
		node2->prev = node1;

		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << "before return 0" << std::endl << std::endl;
#endif
	return 0;
	// 我就不该在写完这个之后点进memory看源码,看不懂就算了还打击自信心
	// 但是现在至少知道它是怎么工作的了
}

#if circularReferenceExample
#include <memory>
class Node {
public:
	std::shared_ptr<Node> next;
	std::shared_ptr<Node> prev;
//	std::weak_ptr<Node> prev; // 使用 weak_ptr 打破循环引用

	~Node() {
		std::cout << "Node destroyed" << std::endl;
	}
};

int main() {
	auto node1 = std::make_shared<Node>();
	auto node2 = std::make_shared<Node>();

	node1->next = node2;
	node2->prev = node1; // 使用 weak_ptr 解决循环引用

	return 0; // Node 对象被正确释放
}
#endif

