//
// Created by fzhinvg on 2025/1/6.
//
#include <exception>
#include <iostream>
#include <cmath>
#include <cassert>
#include <utility>


class myException : public std::exception
{
private:
	unsigned int _error_code;
	std::string _error_message;
public:
	myException(unsigned int error_code, std::string error_message)
			: _error_code(error_code), _error_message(std::move(error_message))
	{};

	[[nodiscard]] const char *what() const noexcept override
	{
		return _error_message.c_str();
	}

};

float divide(float dividend, float divisor)
{
	if (divisor == 0)
//		return std::numeric_limits<float>::quiet_NaN();
//		throw std::runtime_error("Divided by zero!");
		throw myException(1000, "Divided by zero!"); // 使用智能指针来释放动态对象，如果有的话，这里没有
	return dividend / divisor;
}

int main()
{
	assert(divide(10.0, 2.0) == 5.0);
//	assert(std::isnan(divide(10.0, 0.0)));
	try
	{
		divide(1.0, 0.0);
	} catch (myException &e) // 专用靠前catch
	{
		std::cout << e.what() << std::endl;
	} catch (std::exception &e) // 通用应该靠后
	{
		std::cout << e.what() << std::endl;
	}
//	catch (...) // 其他所有类型的异常，但是不太合理，
//				// 如果不能处理那就没必要抓取这个exception
//				// 应该让能处理它的代码块catch
//	{
//		std::cout << "Unknown exception occurred." << std::endl;
//		throw;
//	}
	return 0;
}
