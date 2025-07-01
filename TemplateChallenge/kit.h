//
// Created by fzhinvg on 2025/1/13.
// 至于为什么不叫utils或者utilities,是因为它们不好念

#ifndef TC_KIT_H
#define TC_KIT_H

#include <functional>
#include <iostream>
#include <chrono>
//#include <ctime>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <source_location>
#include <mutex>
#include <fstream>
#include <cxxabi.h>


namespace kit // 哦,我的老天.这样简直酷到爆炸
{
    // 测量运行时间
    template<typename Func_t, typename... Args_t>
    void measureExecutionTime(Func_t func, Args_t &&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        func(std::forward<Args_t>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; // 时间的数值部分用double存储,单位制用ms
        std::cout << "Function executed in " << duration.count() << " ms" << std::endl;
    }

    // 时间戳
    void timestamp(const std::string &message = "") {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::cout << "[" << std::put_time(std::localtime(&in_time_t), "%H:%M:%S") // 时间模板年月日部分: fmt: %Y-%m-%d
                //<< "." << std::setfill('0') << std::setw(3) << ms.count()  // 毫秒位
                << "] " << message << " "; //<< std::endl; // 是否换行
    }

    void timestamp_ms(const std::string &message = "") // 这个是微秒表
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

        std::cout << "[" << std::put_time(std::localtime(&in_time_t), "%H:%M:%S") // 时间模板年月日部分: fmt: %Y-%m-%d
                << "." << std::setfill('0') << std::setw(6) << us.count() // 微秒位
                << "] " << message << " "; //<< std::endl; // 是否换行
    }

    class TimeTracker // 四道铺沃趣
    {
    private:
        std::chrono::high_resolution_clock::time_point start_time_point;

    public:
        TimeTracker() : start_time_point(std::chrono::high_resolution_clock::now()) {
        }

        void start() {
            start_time_point = std::chrono::high_resolution_clock::now();
        }

        [[nodiscard]] time_t getElapsedTime() const // ms
        {
            auto end_time_point = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time_point - start_time_point).count();
            return elapsed;
        }
    };

    class Log {
    private:
        //	std::string _timestamp;
        std::string _tag;
        std::string _filename;
        std::string _code_line;
        std::string _message;

        [[nodiscard]] static std::string timestampMs() {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            struct tm tm_buf{};
#if defined(_WIN32)
            localtime_s(&tm_buf, &in_time_t);
#else
			localtime_r(&in_time_t, &tm_buf);
#endif

            std::stringstream ss;
            ss << "[" << std::put_time(&tm_buf, "%H:%M:%S")
                    << "." << std::setfill('0') << std::setw(3) << ms.count()
                    << "]";
            return ss.str();
        }

    public:
        std::string makeLog() {
            return std::format("{} [{}] [{}:{}] - {}",
                               timestampMs(), _tag, _filename, _code_line, _message);
        };

        Log(std::string tag,
            std::string message,
            const std::source_location &loc) : _tag(std::move(tag)),
                                               _filename(loc.file_name()),
                                               _code_line(std::to_string(loc.line())),
                                               _message(std::move(message)) {
        }
    };

    void log(const std::string &tag, const std::string &message,
             const std::source_location &loc = std::source_location::current()) {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> lock(log_mutex);

        Log log{tag, message, loc};
        std::cout << log.makeLog() << std::endl; // 不要过早地开始优化
    }

    void log(const std::string &tag, const std::string &message,
             const std::filesystem::path &filepath,
             const std::source_location &loc = std::source_location::current()) {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> lock(log_mutex);

        //	static std::ofstream logger{filepath, std::ios::app};
        std::ofstream logger{filepath, std::ios::app}; // <- 我也忘了我当时是咋想的,应该是为了可以使用多个log文件存储信息
        if (logger.is_open()) {
            Log log{tag, message, loc};
            std::string str = log.makeLog();
            logger << str << '\n' << std::flush; // <-
        } else {
            std::cerr << "log file does not exist" << std::endl;
        }
    }

    template<typename T>
    void printSignature(T &&) {
#if defined(__clang__) || defined(__GNUC__)
        std::cout << __PRETTY_FUNCTION__ << '\n'; // GCC/Clang
#elif defined(_MSC_VER)
        std::cout << __FUNCSIG__ << '\n';          // MSVC
#endif
    }

    template <typename T>
std::string getTypeName() {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
        // 编译器特定实现，保留完整类型信息
#if defined(__clang__) || defined(__GNUC__)
        constexpr auto prefix_len = sizeof("std::string getTypeName() [T = ");
        constexpr auto suffix_len = sizeof("]") - 1;
        const char* pretty_func = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
        constexpr auto prefix_len = sizeof("__cdecl getTypeName<") - 1;
        constexpr auto suffix_len = sizeof(">(void)") - 1;
        const char* pretty_func = __FUNCSIG__;
#endif

        const size_t total_len = std::strlen(pretty_func);
        if (total_len <= prefix_len + suffix_len) return "";

        return {pretty_func + prefix_len,
                           total_len - prefix_len - suffix_len};
#else
        // 通用实现 (可能丢失引用/cv信息)
        return typeid(T).name();
#endif
    }

    template <typename T>
    std::string getTypeName(T&&) {
        return getTypeName<T>();
    }

}


#endif //TC_KIT_H
