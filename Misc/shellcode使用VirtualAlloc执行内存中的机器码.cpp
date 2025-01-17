//
// Created by fzhinvg on 2025/1/17.
//
// 这个文件是之前"将exe读进内存然后执行指定位置的函数"的解惑
// 现在我知道了这玩意儿到底应该叫什么,以及运行原理
// 也知道了为什么太复杂的函数(比如链接到的函数)我现在还无法执行
// 还有对应的os层是怎么处理和完成这件事的,这对于目前而言已经足够了
#include <iostream>
#include <windows.h>
#include <cstdint>
#include <vector>
//#include <winsock2.h> // 此处无用,我只是比较感兴趣
// 为什么叫虚拟内存:
// 虚拟内存是一种内存管理技术,允许操作系统为每个进程提供一个独立的 连续的地址空间
// 这种技术使得每个进程可以认为自己拥有整个系统的内存,而实际上这些地址只是虚拟的,
// 由操作系统映射到实际的物理内存(RAM)或磁盘上的页面文件

void openPowershell() // 此内部先不做解释
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// 初始化 STARTUPINFO 结构
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	char commandLine[] = "powershell.exe";

	// 启动命令提示符
	if ((CreateProcess(nullptr,           // 没有指定可执行文件名
					   commandLine,    // 命令行参数
					   nullptr,           // 默认进程安全属性
					   nullptr,           // 默认线程安全属性
					   FALSE,          // 不继承句柄
					   0,              // 创建标志
					   nullptr,           // 使用父进程的环境变量
					   nullptr,           // 使用父进程的当前目录
					   &si,            // 指向 STARTUPINFO 结构的指针
					   &pi)            // 指向 PROCESS_INFORMATION 结构的指针
		) == 0)
	{
//		std::cerr << "CreateProcess failed (" << GetLastError() << ")" << std::endl;
		return;
	}

	// 等待命令提示符退出
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 关闭进程和线程句柄
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

//	std::cout << "Command prompt exited" << std::endl;
}

int main()
{
	// 示例的 shellcode，返回整数 42
	unsigned char shellcode[] = {
			0xb8, 0x2a, 0x00, 0x00, 0x00,  // mov eax, 42
			0xc3                           // ret
	};
	// 示例的 shellcode，返回整数 69
	std::vector<uint8_t> shellcode_ = {
			0xb8, 0x45, 0x00, 0x00, 0x00,  // mov eax, 69
			0xc3                           // ret
	};

	// VirtualAlloc 分配虚拟内存

	// lpAddress 指向分配的内存的起始地址,nullptr表示由os自行决定.如果指定一个位置,函数会尝试从这个地址开始分配内存

	// dwSize 要分配的内存块大小,通常是页的倍数(好像是4kb)

	// flAllocationType 分配类型,指定内存分配的类型,可以是以下一个或多个值的组合
	// MEM_COMMIT: 分配实际物理内存,并将内存页状态设置为已提交
	// MEM_RESERVE: 预留一块虚拟地址空间,但不分配物理内存.必须在使用 MEM_COMMIT 之前调用此标志
	// MEM_RESET: 表示调用方要重置指定内存页的内容.页面在访问时会被清零
	// MEM_TOP_DOWN: 从最高可用地址向下分配内存

	// flProtect 内存保护属性,指定内存保护属性,可以是以下一个或多个值的组合
	//PAGE_READONLY: 内存区域为只读
	//PAGE_READWRITE: 内存区域可读可写
	//PAGE_EXECUTE: 内存区域可执行,但不可读写
	//PAGE_EXECUTE_READ: 内存区域可执行可读,但不可写
	//PAGE_EXECUTE_READWRITE: 内存区域可执行可读写

	void *exec = VirtualAlloc(nullptr,
							  sizeof(shellcode),
							  MEM_COMMIT,
							  PAGE_EXECUTE_READWRITE);
	void *exec_ = VirtualAlloc(nullptr,
							   shellcode_.size(),
							   MEM_COMMIT,
							   PAGE_EXECUTE_READWRITE);

	// 将 shellcode 复制到可执行内存中
	memcpy(exec, shellcode, sizeof(shellcode));
	memcpy(exec_, shellcode_.data(), shellcode_.size());

	// 定义函数指针类型
	typedef int (*func)();
	using func_t = int (*)();

	auto func_ = reinterpret_cast<func_t >(exec_);

	// 执行 shellcode 并获取返回值
	int result = ((func) exec)();
	int result_ = func_();

	VirtualFree(exec, 0, MEM_RELEASE);
	VirtualFree(exec_, 0, MEM_RELEASE);

	std::cout << "Shellcode returned: " << result << " " << result_ << '\n' << std::endl;

//	executionShellcode(openPowershellCode);

	return 0;
}

// 再深究就属于逆向范畴了,但是我目前的能力不足以支撑我探究下去,容易走火入魔
void executionShellcode(const std::vector<uint8_t> &shellcode)
{
	void *memory = VirtualAlloc(nullptr,
								shellcode.size(),
								MEM_COMMIT,
								PAGE_EXECUTE_READWRITE);

	if (memory == nullptr)
	{
		std::cerr << "Memory allocation failed with error: " << GetLastError() << std::endl;
		return;
	}

	memcpy(memory, shellcode.data(), shellcode.size());

	// void openPowershell()

	using func_t = void (*)();

	auto func = reinterpret_cast<func_t>(memory);

	std::cout << "Executing shellcode..." << std::endl;
	func();
	std::cout << "Shellcode execution completed." << std::endl;

	VirtualFree(memory, 0, MEM_RELEASE);

}
