//
// Created by fzhinvg on 2025/1/9.
//
#include <iostream>
#include <windows.h>
#include <string>
#include <cstdint>

#define CLIENT false
#define CUSTOM_STRUCTURE false

int main()
{
	// HANDLE -> void *
	HANDLE hPipe = CreateNamedPipe(
			TEXT("\\\\.\\pipe\\MyNamedPipe"), // \\.\pipe\ 是固定部分,用于指定命名管道路径 \\.\ 表示本地计算机 pipe 表示管道命名空间
			PIPE_ACCESS_DUPLEX, // 管道访问模式 此处表示双向,可以读写
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // 数据类型为字节流;读取模式为字节模式;操作模式为阻塞模式,示当没有数据可读时,读取操作将阻塞,直到有数据可读
			1, // 管道的最大实例数为 1
			1024, // 输出缓冲区大小,1024字节
			1024, // 输入缓冲区大小,1024字节
			0, // 超时时间,0表示默认超时时间
			nullptr // 安全属性,nullptr表示默认安全属性
	);

	// 检验管道句柄是否正确生成
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		std::cerr << "CreateNamedPipe failed." << std::endl;
		return 1;
	}

	std::cout << "Waiting for client to connect..." << std::endl;
	// 等待客户端连接到管道
	if (!ConnectNamedPipe(hPipe, nullptr))
		// lpOverlapped 指向 OVERLAPPED 结构的指针,用于异步操作,如果进行同步操作,可以传入nullptr
	{
		std::cerr << "ConnectNamedPipe failed." << std::endl;
		CloseHandle(hPipe);
		return 1;
	}

	std::cout << "Client connected." << std::endl;
	std::string input_message;
	std::cout << "Enter a message to send: ";
	std::getline(std::cin, input_message);

	DWORD bytesWritten; // DWORD 是无符号32位整数类型 写入的字节数
	unsigned long bytesWritten_; // DWORD 是无符号32位整数类型
	WriteFile(hPipe, // 文件或设备的句柄
			  input_message.c_str(), // 指向要写入的数据缓冲区的指针
			  input_message.size() + 1, // 要写入的字节数 '\0'确保写入完整字符串所以加一 所以实际写入可用容积为1023
			  &bytesWritten_, // 指向写入的字节数的指针
			  nullptr); // 指向 OVERLAPPED 结构的指针,异步操作时使用

//	const char *msg = "i'll be your anchor";
//	DWORD bytesWritten;
//	WriteFile(hPipe,
//			  msg,
//			  strlen(msg) + 1,
//			  &bytesWritten,
//			  nullptr);

	CloseHandle(hPipe); // 使用完句柄需要关闭
	return 0;
}

#pragma region clientDemo
#if CLIENT

#include <iostream>
#include <windows.h>

int main()
{
	// 服务端代码使用 CreateNamedPipe 来创建一个新的命名管道
	// 而客户端代码使用 CreateFile 来连接到一个 已存在 的命名管道
	HANDLE hPipe = CreateFile(
			TEXT("\\\\.\\pipe\\MyNamedPipe"), // 名称需要匹配
			GENERIC_READ | GENERIC_WRITE, // 指定读写访问权限
			0, // 不共享该文件或设备
			nullptr, // 使用默认安全属性
			OPEN_EXISTING, // 打开已存在的管道
			0, // 不使用任何文件属性或标志
			nullptr // 不使用模板文件句柄
	);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		std::cerr << "CreateFile failed." << std::endl;
		return 1;
	}

	char buffer[1024];
	DWORD bytesRead; // 定义一个变量，用于存储实际读取的字节数
	if (ReadFile(hPipe,
				 buffer,
				 sizeof(buffer),
				 &bytesRead,
				 nullptr))
	{
		std::cout << "Client received: " << buffer << std::endl;
	}

	CloseHandle(hPipe);
	return 0;
}
#endif
#pragma endregion

#pragma region customStructure
#if CUSTOM_STRUCTURE

// 序列化和发送
#include <iostream>
#include <windows.h>

struct MyStruct {
	int id;
	char name[50];
	double value;
};

int main() {
	HANDLE hPipe = CreateNamedPipe(
			TEXT("\\\\.\\pipe\\MyNamedPipe"),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			1,
			sizeof(MyStruct),
			sizeof(MyStruct),
			0,
			NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "CreateNamedPipe failed." << std::endl;
		return 1;
	}

	std::cout << "Waiting for client to connect..." << std::endl;
	if (!ConnectNamedPipe(hPipe, NULL)) {
		std::cerr << "ConnectNamedPipe failed." << std::endl;
		CloseHandle(hPipe);
		return 1;
	}

	MyStruct data = {1, "Example", 3.14};
	DWORD bytesWritten;
	WriteFile(hPipe, &data, sizeof(data), &bytesWritten, NULL);

	CloseHandle(hPipe);
	return 0;
}

// 接收和反序列化
#include <iostream>
#include <windows.h>

struct MyStruct {
	int id;
	char name[50];
	double value;
};

int main() {
	HANDLE hPipe = CreateFile(
			TEXT("\\\\.\\pipe\\MyNamedPipe"),
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "CreateFile failed." << std::endl;
		return 1;
	}

	MyStruct data;
	DWORD bytesRead;
	if (ReadFile(hPipe,
				 &data,
				 sizeof(data),
				 &bytesRead,
				 nullptr))
	{
		std::cout << "Client received:" << std::endl;
		std::cout << "ID: " << data.id << std::endl;
		std::cout << "Name: " << data.name << std::endl;
		std::cout << "Value: " << data.value << std::endl;
	}

	CloseHandle(hPipe);
	return 0;
}


#endif
#pragma endregion
