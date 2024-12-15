//
// Created by fzhinvg on 2024/12/4.
//
#include <iostream>
#include <string>
#include <vector>

#define DFA 1

#if DFA

class EmailDFA// 所谓自动机就是输入一个序列，机器状态随着序列转移，直到序列完成，就可以观察到最终状态，从而判断这个序列是否匹配某种模板。
{
public:
	enum State
	{// username@domain.tld
		STATE_INITIAL,
		STATE_USERNAME,
		STATE_AT,
		STATE_DOMAIN,
		STATE_DOT,
		STATE_TLD,
		STATE_INVALID,
		STATE_VALID,// 这个VALID是多余的，但是可读性更强
	};// 枚举在类中定义后，它们的值属于类的作用域，而不是每个对象的实例。

	State state = STATE_INITIAL;

	void transition(char c)// 好像确实没做空白字符的转移，但是cin直接遇到space就结束又弥补了这一点
	{
		switch (state)
		{
			case STATE_INITIAL:
				state = std::isalnum(c) ?
						STATE_USERNAME : STATE_INVALID;
				break;
			case STATE_USERNAME:
				state = (std::isalnum(c) || c == '.' || c == '_') ?
						STATE_USERNAME : (c == '@') ?
										 STATE_AT : STATE_INVALID;
				break;
			case STATE_AT:
				state = std::isalnum(c) ?
						STATE_DOMAIN : STATE_INVALID;
				break;
			case STATE_DOMAIN:
				state = std::isalnum(c) ?
						STATE_DOMAIN : (c == '.') ?
									   STATE_DOT : STATE_INVALID;
				break;
			case STATE_DOT:
//				state = std::isalpha(c) ?
//						STATE_TLD : STATE_INVALID;
//				break;// 两个完全一致的分支
			case STATE_TLD:
				state = std::isalpha(c) ?
						STATE_TLD : STATE_INVALID;// 这里tld只检查是不是alpha显然不是很严谨，但是无所谓，毕竟只是个简单的玩具代码
				break;
			default:
				state = STATE_INVALID;
				break;
		}
	}

	bool isValidEmail(const std::string &input)
	{
		state = STATE_INITIAL;
		for (char c: input)
		{
			transition(c);// 我并不认为这种反复起一个栈帧的情况是好的
			if (state == STATE_INVALID)
			{
				return false;
			}
		}
		state = (state == STATE_TLD) ?
				STATE_VALID : STATE_INVALID;

		return state == STATE_VALID;
	}

#if 0 // 直接嵌入循环，就不需要反复调用函数了
	bool isValidEmail(const std::string& input) {
		state = STATE_INITIAL;
		for (char c : input) {
			switch (state) {
				case STATE_INITIAL:
					state = std::isalnum(c) ? STATE_USERNAME : STATE_INVALID;
					break;
				case STATE_USERNAME:
					state = (std::isalnum(c) || c == '.' || c == '_') ? STATE_USERNAME : (c == '@') ? STATE_AT : STATE_INVALID;
					break;
				case STATE_AT:
					state = std::isalnum(c) ? STATE_DOMAIN : STATE_INVALID;
					break;
				case STATE_DOMAIN:
					state = std::isalnum(c) ? STATE_DOMAIN : (c == '.') ? STATE_DOT : STATE_INVALID;
					break;
				case STATE_DOT:
				case STATE_TLD:
					state = std::isalpha(c) ? STATE_TLD : STATE_INVALID;
					break;
				default:
					state = STATE_INVALID;
					break;
			}
			if (state == STATE_INVALID) {
				return false;
			}
		}
		state = (state == STATE_TLD) ? STATE_VALID : STATE_INVALID;
		return state == STATE_VALID;
	}
#endif
};

void runTests()// 是时候奖励自己一个单元测试用的库了
{
	EmailDFA dfa;

	std::vector<std::pair<std::string, bool>> testCases = {
			{"test@example.com",    true},
			{"user.name@domain.co", true},
			{"user@domain.com",     true},
			{"user@domain",         false},
			{"@domain.com",         false},
			{"user@.com",           false},
			{"user@domain.",        false},
			{"",                    false},
			{"invalid-email",       false},
			{"user@domain.c",       true}, // 适用于不严格检查TLD的情况
	};

	for (const auto &testCase: testCases)
	{
		const std::string &email = testCase.first;
		bool expected = testCase.second;
		bool result = dfa.isValidEmail(email);

		std::cout << "Testing: " << email << " | Expected: " << std::boolalpha << expected << " | Result: " << result
				  << " | "
				  << (result == expected ? "Pass" : "Fail") << std::endl;
	}
}

int main()
{
//	EmailDFA dfa;
//	std::string input;
//
//	std::cout << "Enter an email: ";
//	std::cin >> input;
//
//	if (dfa.isValidEmail(input))
//	{
//		std::cout << "Valid email." << std::endl;
//	} else
//	{
//		std::cout << "Invalid email." << std::endl;
//	}

	runTests();

	return 0;
}

#endif
// 我决定放过自己，先不看NFA