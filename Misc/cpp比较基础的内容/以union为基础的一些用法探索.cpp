//
// Created by fzhinvg on 2024/12/28.
//
#include <iostream>
#include <cstdint>

union Data
{
	uint32_t intVal;
	float floatVal;
	uint8_t bytes[4];
};

union FloatInt
{
	float f;
	uint32_t i;
}; // 惰性转换 快速地重新解释一个数据类型的位模式

struct Message
{
	enum data_t // int 4 传统枚举底层类型通常是int,但不可显式指定
	{
		intType,
		floatType,
		charType
	} dataType;
	union
	{
		int intValue;
		float floatValue;
		char charValue;
	} data; // 4
//	int i = intType; // 传统枚举量可以隐式转换 枚举类是强类型就需要显示转换
}; // 8

union Register // 硬件寄存器操作
{
	uint32_t value;
	struct
	{
		uint32_t flag1: 1;
		uint32_t flag2: 1;
		uint32_t field1: 6;
		uint32_t field2: 8;
		uint32_t reserved: 16; // 1+1+6+8+16=32 简直是天才 这样做就没必要繁复设计struct的赋值逻辑了
	};
}; // spotlight

struct Packet
{
	uint16_t header; // 2+2=4
	union
	{
		struct
		{
			uint8_t type;
			uint8_t length;
		} control; // 2
		struct
		{
			uint32_t address; // 4
			uint16_t port; // 2+2=4
		} data; // 8
	} content;
};// 8+4=12

int main()
{
	Data data{};

	data.intVal = 0x41424344;
	std::cout << "As integer: " << data.intVal << std::endl;
	std::cout << "As float: " << data.floatVal << std::endl;
	std::cout << "As bytes: " << std::hex
			  << static_cast<uint8_t>(data.bytes[0]) << " "
			  << static_cast<uint8_t>(data.bytes[1]) << " "
			  << static_cast<uint8_t>(data.bytes[2]) << " "
			  << static_cast<uint8_t>(data.bytes[3]) << std::dec << std::endl;

	FloatInt value{1.23f};
	std::cout << "As float: " << value.f << std::endl;
	std::cout << "As uint32_t: " << value.i << std::endl;

	Message msg{};
	msg.dataType = Message::data_t::intType;
//	msg.dataType = static_cast<Message::data_t>(1);
	msg.data.intValue = 42;
	std::cout << "Message data type: " << msg.dataType << std::endl;
	std::cout << "Message data: " << msg.data.intValue << std::endl;

	Register reg{};
	reg.value = 0x12345678;
	std::cout << "Flag1: " << std::hex << reg.flag1 << std::endl;
	std::cout << "Reserved: " << reg.reserved << std::dec << std::endl;

	Packet pkt{};
	pkt.header = 0xABCD;
	pkt.content.control.type = 1;
	pkt.content.control.length = 255;
	std::cout << "Packet type: " << static_cast<int>(pkt.content.control.type) << std::endl;
	std::cout << "Packet length: " << static_cast<int>(pkt.content.control.length) << std::endl;
	pkt.content.data.address = 0xfeeeeeee;
	pkt.content.data.port = 0xbaad; // 此处使用断点可以查看字节对齐填充的00

	return 0;
}
// 我突然意识到union和struct和基础类型位字段控制,可以做到非常底层的数据控制
