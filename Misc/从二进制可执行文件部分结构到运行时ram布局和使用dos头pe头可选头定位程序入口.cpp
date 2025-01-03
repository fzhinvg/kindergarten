//
// Created by fzhinvg on 2025/1/3.
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <vector>
#include <sstream>

#define DOSDEMO false

#if DOSDEMO
// DOS 头结构
struct IMAGE_DOS_HEADER
{
	// 魔数 MZ->标识这是一个有效的 DOS 可执行文件头
	// uint16_t == WORD
	uint16_t e_magic;

	uint16_t e_cblp;     // 文件最后一页的字节数
	uint16_t e_cp;       // 文件页数

	// 重定位条目数 -> DOS 可执行文件在加载时需要修正的内存地址数。
	uint16_t e_crlc;

	// 段落头的大小 以 16 字节为单位->用于确定实际代码段的位置
	uint16_t e_cparhdr;

	uint16_t e_minalloc; // 最小额外段数->确定程序运行时的最低内存需求
	uint16_t e_maxalloc; // 最大额外段数->确定程序可以占用的最大内存量

	// 初始（相对）SS值,初始堆栈段的段地址->用于设置程序的初始堆栈段
	uint16_t e_ss;
	// 初始SP,值初始堆栈指针->用于设置程序的初始堆栈指针
	uint16_t e_sp;

	uint16_t e_csum;     // 校验和

	// 初始IP值,初始指令指针->用于设置程序的入口点地址
	uint16_t e_ip;
	// 初始（相对）CS值,初始代码段的段地址->用于设置程序的初始代码段
	uint16_t e_cs;

	// 重定位表的文件地址->用于定位重定位表的位置
	uint16_t e_lfarlc;

	uint16_t e_ovno;     // 覆盖编号->用于支持 DOS 覆盖（overlay）技术，允许程序分段加载
	uint16_t e_res[4];   // 保留字->在当前实现中未使用，但为未来扩展保留

	uint16_t e_oemid;    // OEM 标识符->标识特定 OEM 版本的 DOS
	uint16_t e_oeminfo;  // OEM 信息->与 e_oemid 一起使用，用于特定 OEM 信息
	uint16_t e_res2[10]; // 保留字->在当前实现中未使用,但为未来扩展保留

	// 新 exe 头的文件地址->用于定位 PE 头的位置,这是 PE 文件格式的关键 0x3c==60 16*2+(10+4)*2==60
	int32_t e_lfanew;
};
#endif

template<typename Func_t, typename... Args_t>
void measureExecutionTime(Func_t func, Args_t &&... args)
{
	auto start = std::chrono::high_resolution_clock::now();
	func(std::forward<Args_t>(args)...);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	std::cout << "Function executed in " << duration.count() << " ms" << std::endl;
}

// PE 文件头结构
struct IMAGE_FILE_HEADER
{
	uint16_t Machine; // 允许操作系统和加载器识别目标处理器架构,在适当的环境中运行文件 --0x8664->x64
	uint16_t NumberOfSections; // 表示Section节表的数量
	uint32_t TimeDateStamp; // 文件创建时间的时间戳
	uint32_t PointerToSymbolTable; // 指向符号表的文件指针偏移 [已弃用] 现代 PE 文件通常不使用该字段
	uint32_t NumberOfSymbols; // 符号表中的符号数量 [已弃用] 现代 PE 文件通常不使用该字段
	uint16_t SizeOfOptionalHeader; // IMAGE_OPTIONAL_HEADER可选头的大小,其中包含了更多与映像加载和运行相关的信息
	uint16_t Characteristics; // 文件属性标志 0x0002可执行文件; 0x2000DLL;...很多反正
};

// PE 可选头结构
struct IMAGE_OPTIONAL_HEADER
{
	uint16_t Magic; // 0x010b PE32 32位 ; 0x020b PE32+ 64位
	uint8_t MajorLinkerVersion;
	uint8_t MinorLinkerVersion; // Linker连接器主次版本号,帮助识别编译和链接文件所用的工具版本
	uint32_t SizeOfCode; // 所有代码节的总大小 单位字节
	uint32_t SizeOfInitializedData; // 表示所有已初始化数据节的总大小 单位字节
	uint32_t SizeOfUninitializedData; // 表示所有未初始化数据节的总大小 单位字节 初始化为零
	// 程序入口点的相对虚拟地址 RVA;
	// 操作系统在加载可执行文件时,会从这个地址开始执行代码;
	// 对于 DLL 文件,这个地址指向 DLL 初始化函数的入口点;
	// 我这里是 e0 14 00 00;在ghidra中对应位置为mainCRTStartup入口
	// 实际入口点地址 = ImageBase + AddressOfEntryPoint => 0x0400000+ 0x000014e0==0x004014e0;
	uint32_t AddressOfEntryPoint; // AddressOfEntryPoint 字段指的是入口点函数的 相对虚拟地址(RVA) ,也就是操作系统加载可执行文件时开始执行的地方

	uint32_t BaseOfCode; // 代码段的起始RVA ;是相对于映像基址Image Base的地址,这个字段指示了代码段在内存中的相对虚拟地址
	// 我感觉这个image base八成是没啥卵用,最后都是os自己分配
	uint64_t ImageBase; // 这里假设是 PE32+ 格式;程序在内存中的首选加载地址,如果该地址不可用,操作系统将加载到其他可用的地址
	// 其他字段省略...

	// 之前遇到的直接把程序机器码读入内存,然后使用指针调用函数,直接使用ghidra中的地址访问,却不是正确地址的原因可能是
	// 地址空间布局随机化(ASLR) Address Space Layout Randomization,但是目前我的水平还不足以分析,只是给自己一个暂时的可能性调查方向
	// 还有可能是:重定位表,PE 文件包含一个重定位表,用于指示哪些地址需要在加载时进行调整;dll文件通常会被重定位,以避免地址冲突,但是dll的可能性可以排除
	// 或者是,程序的代码段 数据段 等需要按照特定的页面对齐方式加载到内存中,这也会导致实际运行地址与编译时地址不一致(页面对齐,节对齐)
	// 目前我还不足以处理这种问题,先记下做个引子.
};

#pragma region somethingInteresting
// PE结构
// 头部 Header
// 节 Sections: .text 代码节,包含可执行代码 ; .data 已初始化的数据节 ; .rdata 只读数据节,包含常量和只读数据
// .bss 未初始化的数据节,通常在程序运行时初始化为零
// .idata 导入表,包含需要加载的动态链接库 DLL 信息
// .edata 导出表,包含程序对外暴露的函数和变量
// .rsrc 资源节,包含应用程序资源如图标 字符串 等
// .reloc 重定位表,包含需要在加载时调整的地址

// 在程序加载和执行的过程中,PE 文件中的不同节Section会被读入内存的不同区域
// 这是os和加载器Loader在加载可执行文件时所做的工作
// 运行时: (    段    作用    运行时ram位置    )
// 代码段 .text
// 存储可执行的代码
// 内存区域:通常加载到可执行代码区域,这个区域有执行权限,防止代码被修改
// 数据段 .data
// 存储已初始化的全局变量和静态变量
// 内存区域:加载到读写数据区域,这个区域有读写权限,允许修改数据
// 只读数据段 .rdata
// 存储常量和只读数据
// 内存区域:加载到只读数据区域,这个区域通常只有读权限,防止数据被修改
// 未初始化数据段 .bss
// 存储未初始化的全局变量和静态变量
// 内存区域:加载到 BSS 区域，这个区域在加载时被初始化为零，有读写权限
// 导入表 .idata
// 存储需要加载的动态链接库 DLL 信息
// 内存区域:加载到导入表区域,供加载器解析并加载所需的 DLL
// 导出表 .edata
// 存储程序对外暴露的函数和变量
// 内存区域:加载到导出表区域,供其他程序调用
// 资源段 .rsrc
// 存储应用程序资源如 图标 字符串 等
// 内存区域:加载到资源区域,供程序使用
// 重定位表 .reloc
// 存储需要在加载时调整的地址
// 内存区域:加载到重定位表区域,供加载器进行地址重定位

// Q:为什么更多接触到的顶多是栈帧和堆内存那种ram形态?
// A:他们的抽象程度不同以适应不同程序员的需求.
//   比如软件工程师就不是很需要关心os级的东西,软件工程师更关注应用层面开发,因此主要学习和使用堆和栈的概念,
//   但是将ram细致划分又是操作系统工程师所需要的,他们则需要深入了解底层内存管理,包括 内存分页 地址重定位 等.以优化系统性能和安全性.
//   逆向?我不知道.

// 细致地看一下ram
//高地址
//+-------------------------+
//|       栈 Stack          |    存储函数调用栈帧,包括局部变量 返回地址和参数
//|                         |    LIFO 后进先出 结构,自动分配和释放
//| ... 向下增长 ...         |    通常在内存的高地址部分,向下增长
//+-------------------------+
//|   栈保护区 Stack Guard    |    防止栈溢出攻击,在栈的末尾设置保护页,当溢出时触发保护机制
//+-------------------------+
//|  共享库 Shared Libraries |
//+-------------------------+
//|    堆 Heap              |    动态分配内存,用于存储动态创建的对象和数据
//|                         |    自由存储区,手动分配和释放 malloc free new delete
//| ... 向上增长 ...         |    通常在内存的低地址部分,向上增长
//+-------------------------+
//|  bss 未初始化数据段        |
//+-------------------------+    Data Segment,存储已初始化的全局变量和静态变量;包括.data节 已初始化数据,和.bss节未初始化数据 运行时初始化为零
//|  data 已初始化数据段       |
//+-------------------------+
//| text 代码段              |    Code Segment 或 Text Segment,存储可执行的代码,通常为只读,以防止代码被修改
//+-------------------------+
//低地址
// 其他: Read-Only Data Segment        存储常量和只读数据,通常为只读,以防止数据被修改
//      Memory-Mapped File Region     用于内存映射文件,将文件内容映射到内存中;提高文件I/O操作的效率,允许文件内容直接在内存中操作
// ...

// C++ 中所指的栈区(Stack)
// 堆区(Heap)
// 全局/静态存储区(Global/Static Storage Area)
// 代码区(Code Segment 或 Text Segment)
// 常量存储区(Constant Storage Area)
// 主要是指程序在运行时的 RAM 中的内存布局 而不是 PE 文件中的部分.

// 其中全局/静态存储区(Global/Static Storage Area)划分为Data Segment中
// 已初始化数据段 (Initialized Data Segment):存储已初始化的全局变量和静态变量.这些变量在程序加载时被初始化为指定的值.这部分对应于.data 节
// 和未初始化数据段(Uninitialized Data Segment 或 .bss 节):存储未初始化的全局变量和静态变量.这些变量在程序加载时会被初始化为零.这部分对应于 .bss 节
//  --Global/Static Storage Area 对应于运行时 RAM 中的 数据段 Data Segment ,进一步细分为 .data 和 .bss，还有只读数据部分
//  常量存储区(Constant Storage Area)通常隶属于 只读数据段(Read-Only Data Segment),为了防止被修改
//  总而言之,各种数据根据其不同的读写权限，会被放置在运行时 RAM 中的 Data Segment 或 Read-Only Data Segment.从内存角度看,再细分就要看是否初始化,跟它们是不是静态量没啥关系
#pragma endregion

template<typename T>
// Specialized version of PE file
void byteHippo(const T *address, size_t size, uintptr_t baseAddress)
{
	const auto bytePointer = reinterpret_cast<const uint8_t *>(address);

	size_t bytesPerLine = 16;
	int addressWidth = sizeof(void *) * 2; // 计算地址宽度，32位系统是8，64位系统是16

	// 使用一个字符串缓冲区来存储所有输出
	std::ostringstream oss;

	// 打印地址位移行
	oss << std::setw(addressWidth + 2) << "  "; // 地址宽度和偏移量分隔
	oss << " ";
	for (size_t offset = 0; offset < bytesPerLine; ++offset)
	{
		oss << std::setw(2) << std::setfill('0') << std::hex << offset;
		if ((offset + 1) % 4 == 0) // 每4个字节分隔
		{
			oss << "   "; // 3sp
		} else
		{
			oss << " "; // 1sp
		}
	}
	oss << '\n';

	for (size_t byteIndex = 0; byteIndex < size; ++byteIndex)
	{
		if (byteIndex % bytesPerLine == 0)
		{
			oss << std::setw(addressWidth) << std::setfill('0') << std::hex
				<< baseAddress + byteIndex << ":  ";
		}
		oss << std::setw(2) << std::setfill('0') << static_cast<int>(bytePointer[byteIndex]);

		if ((byteIndex + 1) % 4 == 0) // 每解析4个字节就合适宽一点,好看
		{
			oss << "   "; // 3sp
		} else
		{
			oss << " "; // 1sp 每个字节就1个用来隔断
		}

		// 检查是不是已经输出了完整的一行16个字节
		if ((byteIndex + 1) % bytesPerLine == 0)
		{
			oss << " ";
			for (size_t charIndex = byteIndex - 15; charIndex <= byteIndex; ++charIndex)
			{
				if (std::isprint(bytePointer[charIndex])) // 1==可打印输出的ascii
				{
					oss << static_cast<char>(bytePointer[charIndex]);
				} else
				{
					oss << "·"; // copilot告诉我这个叫中点,十分之十一优雅
				}
			}
			oss << '\n';
		}
	}

	// 处理最后一行不足16字节的情况
	size_t remaining = size % bytesPerLine;
	if (remaining > 0) // 此时光标在最后一个被处理的字节的位置,后面就是填充
	{
		// 用 "  " 填充不足的部分
		for (size_t i = 0; i < (bytesPerLine - remaining); ++i)
		{
			oss << "  "; // 2sp 字节填充
			if ((i + remaining + 1) % 4 == 0)
			{
				oss << "   "; // 3sp 分隔好看
			} else
			{
				oss << " "; // 1sp 字节间距
			}
		}
		oss << " "; // 开始翻译成ascii
		for (size_t i = size - remaining; i < size; ++i)
		{
			if (std::isprint(bytePointer[i]))
			{
				oss << static_cast<char>(bytePointer[i]);
			} else
			{
				oss << "·";
			}
		}
		oss << '\n';
	}

	std::cout << oss.str() << std::dec;
//	measureExecutionTime([&oss]{std::cout << oss.str() << std::dec;});
}

// 读取 PE 文件并输出二进制数据
void PEslayer(const std::string &filename)
{
	std::ifstream file(filename, std::ios::binary); // 以二进制模式打开文件
	if (!file)
	{
		std::cerr << "Unable to open file: " << filename << std::endl;
		return;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// 读取文件数据
	std::vector<uint8_t> fileContent(fileSize); // 上面是为了把整个exe的二进制数据存到一个vector里
	file.read(reinterpret_cast<char *>(fileContent.data()), fileSize);
	file.close(); // 关闭文件

	// 下面一整坨都是为了获取ImageBase来匹配byteHippo的输出
	if (!fileContent.empty())
	{
		// 解析 PE 文件，获取映像基址 ImageBase
		const uint32_t peHeaderOffset = *reinterpret_cast<const uint32_t *>(fileContent.data() +
																			0x3C); // 请看DOSDEMO中struct IMAGE_DOS_HEADER的注释
		if (peHeaderOffset + sizeof(uint32_t) + sizeof(IMAGE_FILE_HEADER) > fileContent.size())
		{
			std::cerr << "Failed to retrieve PE header offset" << std::endl;
			return;
		} // 合理性检验

		const uint32_t *signature = reinterpret_cast<const uint32_t *>(fileContent.data() + peHeaderOffset);
		if (*signature != 0x00004550)
		{ // 检查 PE 签名 "PE\0\0"
			std::cerr << "Invalid PE signature" << std::endl;
			return;
		} // 合理性检验

		// 为什么要多加4个字节的位移?
		// PE文件头结构:
		//    1.dos头
		//    2.PE签名(就是这个)
		//    3.文件头 IMAGE_FILE_HEADER ,实际上第一个字段是 Machine
		//    4.可选头 IMAGE_OPTIONAL_HEADER
		const uintptr_t optionalHeaderOffset = peHeaderOffset + sizeof(uint32_t) + sizeof(IMAGE_FILE_HEADER);
		if (optionalHeaderOffset + offsetof(IMAGE_OPTIONAL_HEADER, ImageBase) + sizeof(uintptr_t) > fileContent.size())
		{
			std::cerr << "Failed to retrieve optional header offset" << std::endl;
			return;
		} // 合理性检验

		const uintptr_t baseAddress = *reinterpret_cast<const uintptr_t *>(fileContent.data() + optionalHeaderOffset +
																		   offsetof(IMAGE_OPTIONAL_HEADER, ImageBase));
		// 说白了上面的程序就是从dos头定位到pe头再定位到可选头,再获取内部的image base字段用于初始化输出需要的地址
		if (baseAddress != 0)
		{
			byteHippo(fileContent.data(), fileContent.size(), baseAddress);
		} else
		{
			std::cerr << "Failed to retrieve PE base address" << std::endl;
		}
	}
}

int main()
{
#if DOSDEMO
	std::ifstream file("pe_sample.exe", std::ios::binary);
	if (!file)
	{
		std::cerr << "Unable to open file" << std::endl;
		return 1;
	}

	// 读取 DOS 头
	IMAGE_DOS_HEADER dosHeader{};
	file.read(reinterpret_cast<char *>(&dosHeader), sizeof(dosHeader));
	if (dosHeader.e_magic != 0x5A4D)
	{ // 检查魔数
		std::cerr << "Invalid DOS Header" << std::endl;
		return 1;
	}

	// 打印 DOS 头信息
	std::cout << "DOS Header Information:" << std::endl;
	std::cout << "Magic number: 0x" << std::hex << dosHeader.e_magic << std::endl;
	std::cout << "Bytes on last page of file: " << std::dec << dosHeader.e_cblp << std::endl;
	std::cout << "Pages in file: " << dosHeader.e_cp << std::endl;
	std::cout << "Relocations: " << dosHeader.e_crlc << std::endl;
	std::cout << "Size of header in paragraphs: " << dosHeader.e_cparhdr << std::endl;
	std::cout << "Minimum extra paragraphs needed: " << dosHeader.e_minalloc << std::endl;
	std::cout << "Maximum extra paragraphs needed: " << dosHeader.e_maxalloc << std::endl;
	std::cout << "Initial (relative) SS value: 0x" << std::hex << dosHeader.e_ss << std::endl;
	std::cout << "Initial SP value: " << std::dec << dosHeader.e_sp << std::endl;
	std::cout << "Checksum: 0x" << std::hex << dosHeader.e_csum << std::endl;
	std::cout << "Initial IP value: 0x" << dosHeader.e_ip << std::endl;
	std::cout << "Initial (relative) CS value: 0x" << dosHeader.e_cs << std::endl;
	std::cout << "File address of relocation table: 0x" << dosHeader.e_lfarlc << std::endl;
	std::cout << "Overlay number: " << std::dec << dosHeader.e_ovno << std::endl;
	std::cout << "OEM identifier: 0x" << std::hex << dosHeader.e_oemid << std::endl;
	std::cout << "OEM information: 0x" << dosHeader.e_oeminfo << std::endl;
	std::cout << "File address of new exe header: 0x" << dosHeader.e_lfanew << std::endl;
#endif
	std::string filename = "pe_sample.exe";
	measureExecutionTime(PEslayer, filename);
//我为什么不直接使用hex editor?🤔
	return 0;
}
