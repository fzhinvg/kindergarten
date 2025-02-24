//
// Created by fzhinvg on 2025/2/25.
//
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread> // for sleep
#include <string>
#include <chrono>

#if _WIN32

#include <Windows.h> // 用于获取管理员权限

#endif


#define symlink_demo false
#define hardlink_demo false

#pragma region symlink_demo
#if symlink_demo

bool isRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	PSID pAdministratorsGroup = nullptr;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(
			&NtAuthority, 2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0,
			&pAdministratorsGroup))
	{
		CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin);
		FreeSid(pAdministratorsGroup);
	}
	return fIsRunAsAdmin;
}

void runAsAdmin()
{
	if (!isRunAsAdmin())
	{
		SHELLEXECUTEINFO sei = {sizeof(sei)};
		sei.lpVerb = "runas";
		sei.lpFile = "kindergarten_local.exe"; // 替换为你的应用程序名
		sei.hwnd = nullptr;
		sei.nShow = SW_NORMAL;

		if (!ShellExecuteEx(&sei))
		{
			std::cerr << "Failed to elevate privileges: " << GetLastError() << std::endl;
			exit(1);
		} else
		{
			exit(0); // 如果提升成功，退出当前实例
		}
	}
}

#endif
#pragma endregion

namespace fs = std::filesystem;

struct Person // 用于测试存储
{
	char name[10];
	int age;
	double salary;
};

void demo();

int main()
{
#if symlink_demo && _WIN32
	runAsAdmin();
#endif
	fs::path dirPath = "example_dir";
	fs::path filePath = dirPath / "person.dat";

	// 创建 fs::create_XXX 返回一个bool用于表示XXX是否成功创建
	if (fs::create_directory(dirPath))
	{
		std::cout << "Directory created: " << dirPath << std::endl;
	} else
	{
		std::cerr << "Failed to create directory or it already exists." << std::endl;
	}

	/*
	 * 不足之处:
	 *
	 * 首先,二进制文件不建议加入换行符,因为平台不同可能并不是都是一个字节'\n'
	 * 但是我感觉可以用对应的in_file.getline()解决,它可以添加第三个参数用于指定分隔符也不一定就要'\n'
	 *
	 * // 未使用 RAII 风格文件流
	 * { // RAII 风格的文件流
	 *		std::ofstream outFile(filePath, std::ios::binary);
	 *		if (outFile) {
	 *			// 操作
	 *		} // 此处流自动关闭
	 *	}
	 *
	 * // 静态断言确保结构体是标准布局
	 * static_assert(std::is_standard_layout<Person>::value, "Person must be standard layout");
	 *
	 * // 使用文件数据没有检查是否文件尾部结束而退出循环
	 * while (inFile.read(reinterpret_cast<char*>(&p), sizeof(Person)))
	 *      { std::cout << "Name: " << p.name << ", Age: " << p.age << std::endl; }
	 *	// 检查是否因文件尾部结束而退出循环
	 *	if (inFile.eof()) {
	 *		std::cout << "End of file reached." << std::endl;
	 *	} else if (inFile.fail()) {
	 *		std::cerr << "Error reading file." << std::endl;
	 *	}
	 *	inFile.close();
	 * */

	// 写入自定义二进制文件
	{
		std::ofstream outFile(filePath, std::ios::binary); // 不指定mode就是默认文本模式,二进制模式不会进行任何转换
		if (outFile.is_open())
		{
			Person p1 = {"Alice", 30, 75000.0};
			Person p2 = {"Bob", 40, 85000.0};

			// reinterpret_cast<const char *>(&p1) -> 要写入的数据指针
			// sizeof(Person) -> 数据的大小
			outFile.write(reinterpret_cast<const char *>(&p1), sizeof(Person));
			outFile.put('\n'); // 写入单个字符
			outFile.write(reinterpret_cast<const char *>(&p2), sizeof(Person));
			outFile.put('\n');
			outFile.close();
			std::cout << "Binary data written to file: " << filePath << std::endl;
		} else
		{
			std::cerr << "Failed to open file for writing." << std::endl;
		}
	}

	// 读取自定义二进制文件
	{
		std::ifstream in_file(filePath, std::ios::binary);
		if (in_file.is_open())
		{
			Person p{};
			std::cout << "Binary data read from file:" << std::endl;
			// reinterpret_cast<char *>(&p) -> 存储读取数据的指针
			// sizeof(Person) -> 要读取的数据大小
			while (in_file.read(reinterpret_cast<char *>(&p), sizeof(Person)))
			{
				in_file.ignore(1); // 跳过指定数量的字符,这里就是换行符
				std::cout << "Name: " << p.name << ", Age: " << p.age << ", Salary: " << p.salary << std::endl;
			}
			in_file.close();
		} else
		{
			std::cerr << "Failed to open file for reading." << std::endl;
		}
	}
	// 这里的序列化方法和反序列化方法只适用于平凡可复制类,意味着不能拥有指针和虚函数(vptr管理的虚函数表)

	// 删除文件
	if (fs::remove(filePath))
	{
		std::cout << "File removed: " << filePath << std::endl;
	} else
	{
		std::cerr << "Failed to remove file." << std::endl;
	}

	// 删除目录
	if (fs::remove(dirPath))
	{
		std::cout << "Directory removed: " << dirPath << std::endl;
	} else
	{
		std::cerr << "Failed to remove directory." << std::endl;
	}

	demo();

	return 0;
}

void demo()
{
//	fs::create_directory()
//		创建一个目录
//	fs::create_directories()
//		创建一个目录链路,如果路径中任意一个中间目录不存在,都会创建
//	fs::create_symlink() -> symbolic link
//		创建一个符号链接(软链接),指向一个文件或目录,相当于fs的一种快捷方式,本质上存储目标路径的字符串
//		如果目标文件被替换或更新,符号链接会自动引用新的文件内容,而不需要手动修改符号链接本身
//		类似于int&和int的关系,实际上使用软链接访问的依然是目标文件,只不过使用了一个
//	fs::create_hard_link()
//		硬链接是文件数据的"多个别名",直接指向文件数据
//		symlink可以跨文件系统,但是hardlink不行,symlink可以连接到一个目录,但是hardlink不可以
//		原文件删除后,hardlink数据保留,直到所有硬链接被删除,symlink则会导致链接失效(悬空)
//		hardlink文件会存储元数据,不会存储用户数据,symlink只存储路径字符串
//		打个比方symlink像是一种可以重新绑定的引用,hardlink像是一种不可以重新绑定的shared_ptr
//		hardlink同样使用fs::remove删除
//		windows下跨分区(卷)创建hardlink会失败
//	fs::create_directory_symlink()
//		专门用于创建 目录的符号链接 Directory Symlink
//		Windows 对符号链接的类型有严格要求,为了兼容跨平台,所以目录和文件的符号链接不能用同一个函数名
//		Windows 会拒绝执行抛出异常,但是 Unix 可能会成功执行

	std::cout << "\n\n\n";

#pragma region symlink_demo
#if symlink_demo
	// symlink软链接演示,windows下需要管理员权限支持,因为符号链接可以指向系统中的任何文件或目录
	// 这里的两个坑,一个是需要使用管理员权限,另一个是需要使用绝对路径
	fs::path target = fs::absolute("example.txt");
	std::ofstream new_file{target, std::ios::binary};
	if (new_file.is_open())
	{
		char buffer[100]{"we got to save the world~"};
		new_file.write(buffer, sizeof(buffer));
		new_file.close();
	} else
	{
		std::cerr << "cannot create target file." << std::endl;
		return;
	}

	fs::path link_dir = fs::absolute("example_link_dir");
	// 符号链接路径
	fs::create_directory(link_dir);
	fs::path link_path = link_dir / "example_link.txt";

	try
	{
		fs::create_symlink(target, link_path); // 符号链接可以指向不存在的路径,但是访问会报错
		std::cout << "symlink created : " << link_path << std::endl;
	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return;
	}

	std::error_code ec;
	auto file_status = fs::status(link_path, ec);
	if (ec)
	{
		std::cerr << ec.message() << std::endl;
	}

	std::ifstream read_from_link_file{link_path,
									  std::ios::binary | std::ios::ate};
	char str[100];
	std::string string{};
	if (read_from_link_file.is_open())
	{
		std::streamsize size = read_from_link_file.tellg();
		// 这里有容易遗忘的一点就是因为需要使用tellg来获取需要读取的size,此时指针在文件尾
		// 读取数据之前需要修改指针的位置
		read_from_link_file.seekg(2, std::ios::beg);
		read_from_link_file.read(str, size);
		std::streamsize count = read_from_link_file.gcount(); // 获取最后一次未格式化输入操作实际读取的字符数量,这里就是读入的字符数
		std::cout << "read through symlink : " << str << " " << count << std::endl;
	} else
	{
		std::cerr << "cannot open symlink file." << std::endl;
	}
	read_from_link_file.close();

	std::this_thread::sleep_for(std::chrono::seconds{3});
	// 用fs::remove()移除单个文件或空文件夹,如果是非空目录它就会返回false,需要使用remove_all()
	if (!fs::remove_all(link_dir, ec))
	{
		std::cerr << ec.message() << std::endl;
	}

#endif
#pragma endregion

#pragma region hardlink_demo
#if hardlink_demo
	fs::path this_dict = fs::current_path();
	fs::path hard_link_file = this_dict / "hardlink.txt";
	fs::path target = fs::absolute("example.txt");
	fs::create_hard_link(target, hard_link_file);
	fs::remove(hard_link_file);
#endif
#pragma endregion

	// create_file这一动作与io相关可以实现逻辑复用
	// std::flush的必要性与异步和多线程有关联
}