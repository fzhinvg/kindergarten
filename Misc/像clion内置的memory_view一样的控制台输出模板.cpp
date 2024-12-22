//
// Created by fzhinvg on 2024/12/19.
//
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cctype>


#if 0
template<typename T>
void byteHippo(T *address, size_t size) // cute and vivid
{
	auto bytePointer = reinterpret_cast<uint8_t *>(address);

	for (size_t i = 0; i < size; ++i)
	{
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[i]);

		if ((i + 1) % 4 == 0)
		{
//			std::cout << "\t";
			std::cout << "    "; // 真是神奇,假如有一天我会回来看这些练习用的玩具代码,希望我还记得为什么用4个空格而不是tab的转义
		} else
		{
			std::cout << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			std::cout << std::endl;
		}
	}
	std::cout << std::dec << std::endl;
}

template<typename T>
void byteHippo(const T &obj)
{
	const auto bytePointer = reinterpret_cast<const uint8_t *>(&obj);
	size_t size = sizeof(obj);
	for (size_t i = 0; i < size; ++i)
	{
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[i]);
		if ((i + 1) % 4 == 0)
		{
			std::cout << "    ";
		} else
		{
			std::cout << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			std::cout << std::endl;
		}
	}
	std::cout << std::dec << std::endl;
}
#endif

#if 0
template<typename T>
void byteHippo(const T *address, size_t size)
{
	const auto bytePointer = reinterpret_cast<const uint8_t *>(address);

	for (size_t i = 0; i < size; ++i)
	{
		if (i % 16 == 0)
		{
			std::cout << std::setw(8) << std::setfill('0') << std::hex << reinterpret_cast<uintptr_t>(bytePointer + i)
					  << ":  ";
		}
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[i]);

		if ((i + 1) % 4 == 0)
		{
			std::cout << "    ";
		} else
		{
			std::cout << " ";
		}

		if ((i + 1) % 16 == 0)
		{
			std::cout << std::endl;
		}
	}
	std::cout << std::dec << std::endl;
}
#endif

// 这里是最终迭代版本
template<typename T>
void byteHippo(const T *address, size_t size)
{
	const auto bytePointer = reinterpret_cast<const uint8_t *>(address);

	size_t bytesPerLine = 16;
	int addressWidth = sizeof(void *) * 2; // 计算地址宽度，32位系统是8，64位系统是16

	for (size_t byteIndex = 0; byteIndex < size; ++byteIndex)
	{
		if (byteIndex % bytesPerLine == 0)
		{
			std::cout << std::setw(addressWidth) << std::setfill('0') << std::hex
					  << reinterpret_cast<uintptr_t>(bytePointer + byteIndex) << ":  ";
		}
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[byteIndex]);

		if ((byteIndex + 1) % 4 == 0) // 每解析4个字节就合适宽一点,好看
		{
			std::cout << "   "; // 3sp
		} else
		{
			std::cout << " "; // 1sp 每个字节就1个用来隔断
		}

		// 检查是不是已经输出了完整的一行16个字节
		if ((byteIndex + 1) % bytesPerLine == 0)
		{
			std::cout << " ";
			for (size_t charIndex = byteIndex - 15; charIndex <= byteIndex; ++charIndex)
			{
				if (std::isprint(bytePointer[charIndex])) // 1==可打印输出的ascii
				{
					std::cout << static_cast<char>(bytePointer[charIndex]);
				} else
				{
					std::cout << "·"; // copilot告诉我这个叫中点,十分之十一优雅
				}
			}
			std::cout << std::endl;
		}
	}

	// 处理最后一行不足16字节的情况
	size_t remaining = size % bytesPerLine;
	if (remaining > 0) // 此时光标在最后一个被处理的字节的位置,后面就是填充
	{
		// 用 "  " 填充不足的部分
		for (size_t i = 0; i < (bytesPerLine - remaining); ++i)
		{
			std::cout << "  "; // 2sp 字节填充
			if ((i + remaining + 1) % 4 == 0)
			{
				std::cout << "   "; // 3sp 分隔好看
			} else
			{
				std::cout << " "; // 1sp 字节间距
			}
		}
		std::cout << " "; // 开始翻译成ascii
		for (size_t i = size - remaining; i < size; ++i)
		{
			if (std::isprint(bytePointer[i]))
			{
				std::cout << static_cast<char>(bytePointer[i]);
			} else
			{
				std::cout << "·";
			}
		}
		std::cout << std::endl;
	}

	std::cout << std::dec << std::endl;
}

// 下面两个变量名没有优化rename,但是逻辑相同
template<typename T>
void byteHippo(const T &obj)
{
	const auto bytePointer = reinterpret_cast<const uint8_t *>(&obj);
	size_t size = sizeof(obj);
	size_t bytesPerLine = 16;
	int addressWidth = sizeof(void *) * 2; // 计算地址宽度，32位系统是8，64位系统是16

	for (size_t i = 0; i < size; ++i)
	{
		if (i % bytesPerLine == 0)
		{
			std::cout << std::setw(addressWidth) << std::setfill('0') << std::hex
					  << reinterpret_cast<uintptr_t>(bytePointer + i) << ":  ";
		}
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[i]);

		if ((i + 1) % 4 == 0)
		{
			std::cout << "   "; // 使用固定数量的空格分隔
		} else
		{
			std::cout << " ";
		}

		if ((i + 1) % bytesPerLine == 0)
		{
			std::cout << " ";
			for (size_t j = i - 15; j <= i; ++j)
			{
				if (std::isprint(bytePointer[j]))
				{
					std::cout << static_cast<char>(bytePointer[j]);
				} else
				{
					std::cout << "·";
				}
			}
			std::cout << std::endl;
		}
	}

	// 处理最后一行不足16字节的情况
	size_t remaining = size % bytesPerLine;
	if (remaining > 0)
	{
		// 用 "--" 填充不足的部分
		for (size_t i = 0; i < (bytesPerLine - remaining); ++i)
		{
			std::cout << "  ";
			if ((i + remaining + 1) % 4 == 0)
			{
				std::cout << "   ";
			} else
			{
				std::cout << " ";
			}
		}
		std::cout << " ";
		for (size_t i = size - remaining; i < size; ++i)
		{
			if (std::isprint(bytePointer[i]))
			{
				std::cout << static_cast<char>(bytePointer[i]);
			} else
			{
				std::cout << "·";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::dec << std::endl;
}

struct BtB
{
	int Sorrow;
	double gives;
	char us;
	bool fighting;
	float power;
};

int main()
{
	int exampleInt = 42;
	double exampleDouble = 2.333;
	std::string exampleString = "The Way of Adventure (feat. クサカアキラ)"; // love it! 应景
	// 35-6+3*6=?
	auto pBtB = new BtB{5, 9.1, 's', true, 100.0};

	std::cout << "Memory of exampleInt:" << std::endl;
	byteHippo(&exampleInt, sizeof(exampleInt));

	std::cout << "Memory of exampleDouble:" << std::endl;
	byteHippo(&exampleDouble, sizeof(exampleDouble));

	std::cout << "Memory of exampleString:" << std::endl;
	byteHippo(exampleString.data(), exampleString.size());

	std::cout << "Memory of example StandardLayout structure:" << std::endl;
	byteHippo(*pBtB);
	byteHippo(pBtB, sizeof(BtB));

	delete pBtB;
	return 0;
}
