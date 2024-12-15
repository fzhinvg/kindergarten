//
// Created by fzhinvg on 2024/12/6.
//
// 太神奇了
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
// 看来我并不是很了解pe文件的结构与windows对他的运行处理机制
// 我想把exe直接读到内存里，我寻思对应机器码没道理不能运行
// 但实际上我只能运行这种没有使用别的需要初始化对象的简单函数
// 复杂一些的内容，应该不太适合我这种新手
using FuncType = int (*)();

// 用于在 vector 中查找特定指令序列
std::vector<uint8_t>::iterator find_sequence(std::vector<uint8_t> &buffer, const std::vector<uint8_t> &sequence)
{
	return std::search(buffer.begin(), buffer.end(), sequence.begin(), sequence.end());
}

int main()
{
	// 读取目标exe文件到 vector 中，目前我还不知道为什么这个0x1390为什么不是正确偏移，在实际运行的时候偏移量是0x790
	/*
	 *  ram:140000000-ram:1400003ff
     *  140001390 b8  47  a5       MOV        EAX ,0x1919a547
     *            19  19
     *  140001395 c3              RET
	 */
	std::ifstream exeFile("target_super_easy.exe", std::ios::binary | std::ios::ate);
	if (!exeFile)
	{
		std::cerr << "Failed to open the file!" << std::endl;
		return 1;
	}

	// 获取文件大小并分配内存
	std::streamsize size = exeFile.tellg();
	exeFile.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if (!exeFile.read(reinterpret_cast<char *>(buffer.data()), size))
	{
		std::cerr << "Failed to read the file!" << std::endl;
		return 1;
	}

	std::cout << buffer.size() << std::endl;

// 调试输出缓冲区的前几字节内容
//	for (unsigned char i: buffer)
//	{
//		printf("%02x ", i);
//	}
//	printf("\n");

	// 定义你要查找的指令序列 (这里是用于定位func位置的机器码，查找自ghidra)
	std::vector<uint8_t> func_sequence = {0xb8, 0x47, 0xa5, 0x19, 0x19, 0xc3};// 后续定位到了之后并没有使用上这里查找到的值

	// 在 buffer 中查找指令序列
	auto it = find_sequence(buffer, func_sequence);
//	if (it != buffer.end())
	{
		// 计算偏移量
//		uintptr_t offset = std::distance(buffer.begin(), it);
//		std::cout << "Found func at offset: 0x" << std::hex << offset << std::endl;

		uintptr_t offset = 0x790;

		// 转换为函数指针并调用
		auto func = reinterpret_cast<FuncType>(buffer.data() + offset);


		DWORD oldProtect;
		if (!VirtualProtect(buffer.data(), buffer.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			std::cerr << "Failed to set memory protection!" << std::endl;
			return 1;
		}

		std::cout << "First 6 bytes of function:\n";
		for (size_t i = 0; i < 6; ++i)
		{
			std::cout << std::hex << std::setw(2) << std::setfill('0')
					  << static_cast<int>(*(reinterpret_cast<uint8_t *>(func) + i)) << " ";
			if ((i + 1) % 16 == 0)
			{ std::cout << "\n"; }
		}
		std::cout << std::dec << std::endl;

//		int we_get = func();
		std::cout << func() << std::endl;
	}
//	else
//	{
//		std::cerr << "Function sequence not found!" << std::endl;
//	}

	return 0;
}
