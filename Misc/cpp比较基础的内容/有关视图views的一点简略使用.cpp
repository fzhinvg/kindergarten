//
// Created by fzhinvg on 2025/7/24.
//
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>


#include "kit.h"

template<typename T>
void show(T view) {
    for (auto item: view) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

// template<typename Rep, typename Period, typename Function>
// void triggerAfter(const std::chrono::duration<Rep, Period> &duration, Function &&func) {
//     std::thread([duration, func = std::forward<Function>(func)]() mutable {
//         std::this_thread::sleep_for(duration);
//         func();
//     }).detach();
// }

template<typename Rep, typename Period>
void setBoolAfter(std::chrono::duration<Rep, Period> delay,
                  std::atomic<bool> &flag,
                  bool targetValue = true) {
    std::thread([delay, &flag, targetValue] {
        std::this_thread::sleep_for(delay);
        flag.store(targetValue);
    }).detach();
}

int main() {
    auto display = [](const char *title, auto view) {
        std::cout << "-" << title << "----------" << std::endl;
        show(std::move(view));
    };
    std::vector<int> numbers{-2, -1, 0, 1, 2, 3, 4};
    show(numbers);

    // 过滤 此处的pred是一个可调用对象
    // auto even = std::views::filter(numbers, [](int a) { return a % 2 == 0; });
    auto even = numbers | std::views::filter([](int a) { return a % 2 == 0; });
    auto square = numbers | std::views::transform([](int n) { return n * n; });
    display("even", even);
    display("square", square);

    auto first3 = numbers | std::views::take(3);
    auto last3 = numbers | std::views::drop(numbers.size() - 3);
    display("first3", first3);
    display("last3", last3);

    // 逆序
    auto reversed = numbers | std::views::reverse;
    display("reversed", reversed);

    // 嵌套展开
    std::vector<std::vector<int> > matrix = {{1, 2}, {3, 4}, {5}};
    auto flattened = matrix | std::views::join;
    display("flattened", flattened);

    // 生成无限序列
    std::cout << "-infinite----------" << std::endl;
    auto infinite = std::views::iota(1);
    auto alive = std::make_shared<std::atomic<bool> >(true);
    setBoolAfter(std::chrono::seconds(4), *alive, false);
    auto iter = infinite.begin();
    while (alive->load()) {
        std::cout << *iter << " " << std::flush;
        ++iter;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    std::cout << std::endl;


    auto limited = std::views::iota(1, 10);
    display("limited", limited);

    auto split_by_comma = [](const std::string &s) {
        return s
               | std::views::split(',')
               | std::views::transform([](auto part) {
                   return std::string_view(part);
               });
    };
    display("split_by_comma", split_by_comma("Don't,wait"));

    // 组合视图
    auto combined = numbers
                    | std::views::filter([](int n) { return n > 0; })
                    | std::views::transform([](int n) { return n * n; })
                    | std::views::take(3); // 管道写法的触发顺序是从左到右的,编译后更像是take(transform(filter(numbers)))
    display("Positive squared first3", combined);


    // 它看起来更像是一种滤镜，让我们看见特定的频率的光，而不拥有光
    // wait 它看起来好像并没有我想象的那么容易，我意识到一个问题
    // transform_view是怎么做到获取转换后的值而不修改原始容器 ???

    //  惰性求值 Lazy Evaluation 在每次访问元素时动态应用转换函数
    auto Lazy_view = numbers | std::views::transform([](int n) {
        return n << 1; // <-- set breakpoint in this line
    });

    // 此时没有计算发生
    // 实际计算发生在迭代时
    display("Lazy Evaluation view", Lazy_view);

    // iota的使用方式
    for (auto i: std::views::iota(0, 4)) {
    std::cout << i << " "; // 0 1 2 3 惰性生成的线性序列工厂
    }

    auto fibonacci = std::views::iota(0, 7)
                     | std::views::transform([](int n) {
                         return round(pow(1.618, n) / 2.236);
                     });

    display("fibonacci", fibonacci);

    using namespace std::views;
    // auto odds = iota(1) | filter([](int x) { return x % 2 != 0; });
    auto odds = iota(-2) | transform([](int x) { return 2 * x - 1; });

    int cnt = 0;
    for (auto item: odds) {
        if (cnt > 10)
            break;
        std::cout << item << " ";
        ++cnt;
    }

    // 这看起来像是个对算法领域很有用的工具,但是我觉的现在的我还不需要深究太多,现在我还是新手状态,我需要囫囵吞枣地阅读,不求甚解的学习,而不是针对一个东西反复深究,执着于钻牛角尖

    return 0;
}
