//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <cstring>

class ShallowCopy
{
public:
	char *data;

	ShallowCopy(const char *str)
	{
		data = new char[strlen(str) + 1];
		strcpy(data, str);
	}

	// 浅拷贝构造函数
	ShallowCopy(const ShallowCopy &other) : data(other.data)
	{}

	~ShallowCopy()
	{
		delete[] data;
	}
};

int main()
{
	ShallowCopy obj1("Hello");
	ShallowCopy obj2 = obj1;  // 使用浅拷贝构造函数
	obj1.data[0] = 'X';

	std::cout << "obj1 data: " << obj1.data << std::endl;
	std::cout << "obj2 data: " << obj2.data << std::endl;  // obj2 受 obj1 修改影响

	return 0;
}

#if 0
#include <iostream>
#include <cstring>

class DeepCopy {
public:
	char* data;
	DeepCopy(const char* str) {
		data = new char[strlen(str) + 1];
		strcpy(data, str);
	}
	// 深拷贝构造函数
	DeepCopy(const DeepCopy& other) {
		data = new char[strlen(other.data) + 1];
		strcpy(data, other.data);
	}
	~DeepCopy() {
		delete[] data;
	}
};

int main() {
	DeepCopy obj1("Hello");
	DeepCopy obj2 = obj1;  // 使用深拷贝构造函数
	obj1.data[0] = 'X';

	std::cout << "obj1 data: " << obj1.data << std::endl;
	std::cout << "obj2 data: " << obj2.data << std::endl;  // obj2 不受 obj1 修改影响

	return 0;
}
#endif