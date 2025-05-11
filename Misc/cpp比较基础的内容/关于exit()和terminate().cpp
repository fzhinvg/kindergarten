//
// Created by fzhinvg on 2025/5/11.
//
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>

void cleanup()
{ std::cout << "function:Cleanup via exit()\n"; }

class Singleton
{
public:
    static Singleton &getInstance()
    {
        static Singleton instance; // 局部静态变量
        return instance;
    }

    ~Singleton()
    {
        std::cout << "Singleton is destroyed\n";
    }

private:
    Singleton() = default; // 私有构造函数
}; // 我已经不知道我为什么要在这里写一个这个了

void detach_thread()
{
    std::cout << "thread sleep..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "This sentence should not be printed." << std::endl;
}
// exit()是进程级终止，会杀死主线程以及所有附属子线程
// 它会析构所有全局/静态对象，但不会析构局部对象（包括主线程和其他线程的栈对象），因此存在泄露风险，这里实际上指的是是指程序级别的逻辑性资源泄漏，而非操作系统层面的物理内存泄漏
// 子线程被强制终止，不会执行清理操作，可能会导致正在io的文件受损

struct GlobalObj
{
    ~GlobalObj()
    { std::cout << "GlobalObj destructor was called\n"; }
} global;

struct HeapObj
{
    ~HeapObj()
    { std::cout << "HeapObj destructor was called\n"; }
};

int main()
{
    std::atexit(cleanup); // 使用此函数注册，exit调用时会自动触发，不需要手动调用，程序自然推出也会调用

    Singleton::getInstance(); // 意义不明，应该是为了说明静态量也会被析构

    std::thread task{detach_thread};
    task.detach();

    struct LocalObj
    {
        ~LocalObj()
        { std::cout << "LocalObj destructor was called\n"; }
    };
    LocalObj local; // stack obj

    auto pHeapObj = new HeapObj{};
//  delete pHeapObj; // 拉屎没关系，os会帮你捡

#pragma region exit_region
    // 触发 terminate()（例如：未捕获异常）
    // throw 1; // or
    // std::terminate();

    // 显式调用 exit()，执行清理
    std::exit(EXIT_SUCCESS);
    // std::exit(EXIT_FAILURE);

    // 显式调用 terminate()（不推荐）
    // std::terminate();
#pragma endregion
}
/*
 * 功能用途：
 * exit()
 * 用于正常终止程序，通常由开发者显式调用
 * 执行清理操作：刷新缓冲区、关闭文件流、调用 atexit 注册的函数，并析构全局/静态对象
 * 接受退出状态码（如 EXIT_SUCCESS 或 EXIT_FAILURE）
 * std::terminate()
 * 用于异常终止程序，通常在以下情况自动触发
 *     未被捕获的异常
 *     noexcept 函数抛出异常
 *     析构函数在栈展开时抛出异常
 * 默认行为是调用 abort()，不保证清理资源（如不析构局部对象，不刷新缓冲区）
 * 可通过 set_terminate() 自定义终止处理函数，但最终仍会终止程序
 *
 * 清理行为：
 * exit()
 * 析构全局和静态对象
 * 调用 atexit 注册的函数
 * 不析构局部对象（栈未展开）
 * std::terminate()：
 * 默认直接调用 abort()，不执行任何清理
 * 栈展开行为由实现决定（C++ 标准未强制要求），可能导致局部对象未析构
 * 全局/静态对象的析构函数可能不会执行
 */