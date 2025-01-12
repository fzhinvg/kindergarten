//
// Created by fzhinvg on 2025/1/12.
//
#include <iostream>
#include <windows.h>
//#include <cstdint>

#define READER false
#define FILE_DEMO false

// 关于为什么File Mapping的翻译是共享内存
// 虽然直译是"文件映射"但它的实际用途和概念更多地与"共享内存"有关,特别是在应用程序间共享数据时
// file mapping是操作系统提供的一种机制,可以将文件的内容映射到进程的内存地址空间,从而实现内存和文件内容的映射
// 本质上是将文件内容映射到进程的虚拟地址空间,使得进程可以像访问内存一样访问文件内容
// 这并不总是意味着整个文件会立即从硬盘读取到 RAM 中,而是根据需要逐步加载
// 我的理解是os对于磁盘io和内存管理的一种封装

// Windows 共享内存命名中,Local\\ 是一个命名空间前缀,表示共享内存对象的作用范围为本地计算机,适用于仅在单台计算机上的多个进程之间共享数据
// 同理,还存在Global\\ 表示共享内存对象在整个系统范围内可见,包括所有会话.适用于多会话环境(终端服务器),需要在多个会话之间共享数据的情况
const char *SHARED_MEMORY_NAME = "Local\\MySharedMemory";
constexpr int SHARED_MEMORY_SIZE = 1024;

// 因为共享内存是由os管理的,所以所有访问该共享内存的进程都会看到同一份数据,而不会出现每个进程有各自副本的问题
int main()
{
	// 创建共享内存对象
	HANDLE hMapFile = CreateFileMapping(
			// 该参数通常用于创建基于页面文件的内存映射文件,不与实际的物理文件关联
			// 如果要映射一个实际存在的文件,这里应该传入文件的句柄(通过 CreateFile 获得)
			INVALID_HANDLE_VALUE,
			// 安全属性指针,指向一个 SECURITY_ATTRIBUTES 结构体,用于指定新对象的安全属性
			// nullptr表示使用默认安全性设置
			nullptr,
			// 保护属性,指定页面文件的保护类型,其他选项包括 PAGE_READONLY(只读) PAGE_EXECUTE(可执行)...
			PAGE_READWRITE,
			// 下面两个数值拼起来就是最大大小,一个64位的无符号整形,单位字节
			0,                      // 最大对象大小 高位 DWORD
			SHARED_MEMORY_SIZE,     // 最大对象大小 低位 DWORD -> ff ff ff ff 就是 4gb
			// 1, // 最大对象大小 高位 DWORD
			// 2147483648, // 最大对象大小 低位 DWORD 换成这俩就是 6gb

			//映射对象的名称,可以用于其他进程打开该共享内存对象,名称应在系统范围内唯一
			SHARED_MEMORY_NAME
	);
	/* h (handle)
	 * dw (DWORD)
	 * lp (long pointer) 至于为什么是长指针,这是历史遗留问题,现代计算机在64位下ptr都是8byte
	 * fl (flag)
	 *
	 * b (boolean)
	 * c (count/character)  表示计数或字符
	 * cb (count of bytes)  表示字节数
	 * ch (character)  表示单个字符
	 * dw (DWORD)  32 位无符号整数,双字
	 * f (flag/float)  表示标志或浮点数,上下文决定
	 * i (integer)
	 * n (number)  表示数量或编号
	 * p (pointer)
	 * psz (pointer to null-terminated string)  表示指向以空字符结尾的字符串的指针
	 * u (unsigned)  表示无符号整数 一个有趣的是:uID 表示无符号整数的标识符
	 * w (word)  字,16 位无符号整数
	 * l (long)
	 * */
	if (hMapFile == nullptr)
	{
		std::cerr << "Could not create file mapping object (" << GetLastError() << ").\n";
		return 1;
	}

	// MapViewOfFile 函数用于将文件映射对象的一个视图映射到调用进程的地址空间,从而使得进程能够访问文件内容或共享内存
	char *pBuf = static_cast<char *>(MapViewOfFile(
			hMapFile,                // 共享内存对象的句柄
			FILE_MAP_ALL_ACCESS,     // 可读写权限
			0,                       // 文件偏移量 (高位 DWORD )
			0,                       // 文件偏移量 (低位 DWORD)  该参数指定从文件的哪个位置开始映射
			SHARED_MEMORY_SIZE       // 要映射的字节数,如果设为 0 则从偏移量开始到文件的结尾进行映射
	)); // 使用char*作为字节指针是一个历史原因,其实我更喜欢uint8_t*

	if (pBuf == nullptr)
	{
		std::cerr << "Could not map view of file (" << GetLastError() << ").\n";
		CloseHandle(hMapFile);
		return 1;
	}

	// 写入数据到共享内存
//	const char *message = "Hello from shared memory!";
	const char *message = "My bank account is 000, oh no";
	CopyMemory(pBuf, message, (strlen(message) + 1) * sizeof(char)); // 类似于c的 memcpy()

	std::cout << "Data written to shared memory.\n";

	std::cin.get();

	// 清零共享内存区域,确保数据安全,说实话,我感觉在逆向面前这些基础的安全措施都没什么用吧,但是可以增加逆向壁垒和成本
	// [optional] (●'◡'●)
	SecureZeroMemory(pBuf, SHARED_MEMORY_SIZE); // 它只是将指定的内存区域清零,而不会改变内存的映射或释放内存资源

	// 解除映射视图
	UnmapViewOfFile(pBuf);
	// 关闭共享内存对象句柄
	CloseHandle(hMapFile); // 这两者都像指针一样使用完要释放虚拟内存和关闭句柄 //共享内存通过文件映射 File Mapping 机制实际上利用了虚拟内存
	return 0;
}

#pragma region reader
#if READER
#include <iostream>
#include <windows.h>

const char *SHARED_MEMORY_NAME = "Local\\MySharedMemory";
const int SHARED_MEMORY_SIZE = 1024;

int main()
{
	// 打开现有的共享内存对象
	HANDLE hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // 可读写权限
			FALSE,                 // 处理继承选项,句柄不继承;句柄继承是指在创建新进程时,父进程的句柄是否会被新进程继承;如果是true,也就是这个句柄的访问权可以下放到子进程的意思
			SHARED_MEMORY_NAME     // 映射对象名称，应与创建时相同
	);

	if (hMapFile == nullptr)
	{
		std::cerr << "Could not open file mapping object (" << GetLastError() << ").\n";
		return 1;
	}

	// 映射文件视图到进程地址空间
	char *pBuf = static_cast<char *>(MapViewOfFile(
			hMapFile,                // 共享内存对象的句柄
			FILE_MAP_ALL_ACCESS,     // 可读写权限
			0,                       // 文件偏移量（高位 DWORD）
			0,                       // 文件偏移量（低位 DWORD）
			SHARED_MEMORY_SIZE       // 要映射的字节数
	));

	if (pBuf == nullptr)
	{
		// GetLastError() 它用于获取当前线程的上一个错误代码
		std::cerr << "Could not map view of file (" << GetLastError() << ").\n";
		CloseHandle(hMapFile);
		return 1;
	}

	// 读取共享内存中的数据
	std::cout << "Data from shared memory: " << pBuf << "\n";

	// 解除映射视图
	UnmapViewOfFile(pBuf);
	// 关闭共享内存对象句柄
	CloseHandle(hMapFile);
	return 0;
}
#endif
#pragma endregion

#pragma region file_demo
#if FILE_DEMO
// 我之前在使用管道的时候好奇过怎么传递自定义struct,因为我当时觉得管道更适合用来传递这种变量,结构之类的,现在使用共享内存我有点好奇能不能共享一整个文件
// 下面是我让大模型帮我生成的,不确保能否使用,仅作参照

// Writer.cpp
#include <iostream>
#include <fstream>
#include <windows.h>

constexpr char *SHARED_MEMORY_NAME = "Local\\CsvSharedMemory";
constexpr int SHARED_MEMORY_SIZE = 4096; // 假设 CSV 文件不会超过 4KB

int main() {
	// 读取 CSV 文件内容
	std::ifstream csvFile("example.csv");
	if (!csvFile.is_open()) {
		std::cerr << "Could not open CSV file.\n";
		return 1;
	}

	std::string csvData((std::istreambuf_iterator<char>(csvFile)),
						 std::istreambuf_iterator<char>());
	csvFile.close();

	// 创建共享内存对象
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0,
		SHARED_MEMORY_SIZE,
		SHARED_MEMORY_NAME
	);

	if (hMapFile == nullptr) {
		std::cerr << "Could not create file mapping object (" << GetLastError() << ").\n";
		return 1;
	}

	// 映射文件视图到进程地址空间
	char *pBuf = static_cast<char *>(MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		SHARED_MEMORY_SIZE
	));

	if (pBuf == nullptr) {
		std::cerr << "Could not map view of file (" << GetLastError() << ").\n";
		CloseHandle(hMapFile);
		return 1;
	}

	// 写入 CSV 文件内容到共享内存
	CopyMemory(pBuf, csvData.c_str(), csvData.size() + 1);

	std::cout << "CSV data written to shared memory.\n";
	std::cin.get();

	// 清零共享内存区域，确保数据安全
	SecureZeroMemory(pBuf, SHARED_MEMORY_SIZE);

	// 解除映射视图
	UnmapViewOfFile(pBuf);
	// 关闭共享内存对象句柄
	CloseHandle(hMapFile);

	return 0;
}

// Reader.cpp
#include <iostream>
#include <windows.h>

constexpr char *SHARED_MEMORY_NAME = "Local\\CsvSharedMemory";
constexpr int SHARED_MEMORY_SIZE = 4096; // 假设 CSV 文件不会超过 4KB

int main() {
	// 打开现有的共享内存对象
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		SHARED_MEMORY_NAME
	);

	if (hMapFile == nullptr) {
		std::cerr << "Could not open file mapping object (" << GetLastError() << ").\n";
		return 1;
	}

	// 映射文件视图到进程地址空间
	char *pBuf = static_cast<char *>(MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		SHARED_MEMORY_SIZE
	));

	if (pBuf == nullptr) {
		std::cerr << "Could not map view of file (" << GetLastError() << ").\n";
		CloseHandle(hMapFile);
		return 1;
	}

	// 读取共享内存中的 CSV 数据
	std::cout << "CSV data read from shared memory: " << pBuf << "\n";

	// 清零共享内存区域，确保数据安全
	SecureZeroMemory(pBuf, SHARED_MEMORY_SIZE);

	// 解除映射视图
	UnmapViewOfFile(pBuf);
	// 关闭共享内存对象句柄
	CloseHandle(hMapFile);

	return 0;
}
#endif
#pragma endregion
