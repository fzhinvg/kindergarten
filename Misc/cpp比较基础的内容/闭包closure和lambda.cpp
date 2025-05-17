//
// Created by fzhinvg on 2025/5/11.
// 闭包closure 闭包类 与 lambda
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

auto outFunc()
{
    int cnt = 0;
    return [=]mutable
    {
        cnt++;
        std::cout << cnt << std::endl;
    }; // 总之其核心理念就是用lambda生成一个包含捕获成员的类，当然，这样的话就没办法隐式转化为函数指针了，要隐式转换成函数指针还是不能捕获任何成员
}

int main()
{
    class Inner
    {
    public:
        static auto func()
        {
            int cnt = 0;
            return [=]mutable
            {
                cnt++;
                std::cout << cnt << std::endl;
            };
        }
    }; // 某种意义上可以用这种写法制造一个伪的 local function

    auto f = outFunc();
    auto fn = Inner::func();
    std::cout << typeid(f).name() << std::endl;
    std::cout << typeid(fn).name() << std::endl;

    void (*log)() =[]
    { std::cout << "hello" << std::endl; }; // 这样就可以隐式转化成一个函数指针
    /*
     * 这里引申出一个问题，关于std::thread的构造，使用函数指针就不必纠结了
     * 实际上std::thread构造使用的是一个可调用对象，也就是说传入的如果是一个对象
     * 那么就需要实现它的operator()方法，否则无法通过编译
     * */
#define TEST true
    class ThreadTest
    {
        int _self_value;
    public:
        explicit ThreadTest(int self_value) : _self_value(self_value)
        {}
    public:
        ThreadTest(const ThreadTest&) = default;
        ThreadTest(ThreadTest&&) = default;

#if TEST
        void operator()() const
        {
            std::cout << _self_value << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds{3});
            std::cout << _self_value + 1 << std::endl;
        }
#endif
    };
    auto obj = std::make_unique<ThreadTest>(0x7fff'ffff);
    std::thread task{*obj};
    if (task.joinable())
    {
        task.join();
    }

    f();
    f();
    f();
    f();


    return 0;
}

// 闭包对象支持移动操作，可安全转移资源所有权
auto create_heavy_closure()
{
    std::vector<int> data(1000); // 大量数据
    return [data = std::move(data)]()
    { /* 使用 data */ }; // 移动捕获，避免拷贝
}

// 可显式管理堆内存，延长捕获变量的生命周期
auto create_closure()
{
    auto ptr = std::make_unique<int>(42);
    return [ptr = std::move(ptr)]()
    { return *ptr; }; // 安全持有 unique_ptr
}

void register_callback(void(*callback)(int))
{ /* ... */ }

void brace()
{
    // 无捕获 lambda 可隐式转换为函数指针，兼容 C 接口或低级 API
    register_callback([](int x)
                      { std::cout << x; }); // 无捕获 lambda → 函数指针
}


