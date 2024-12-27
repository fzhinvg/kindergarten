//
// Created by fzhinvg on 2024/12/27.
//
#include <cstdint>
#include <iostream>

enum class Time : uint16_t
{
	morning = 65531,
	noon,
	afternoon,
	evening,
	midnight
};

enum class Permission : uint8_t
{
	None = 0b0000'0000,
	Create = 0b0000'0001,
	Update = 0b0000'0010,
	Delete = 0b0000'0100,
	Normal = Create | Update,
	Administrator = Create | Update | Delete
};

inline Permission operator|(Permission lhs, Permission rhs)
{
	return static_cast<Permission>(
			static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
	);
}

inline Permission operator&(Permission lhs, Permission rhs)
{
	return static_cast<Permission>(
			static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
	);
}


struct BitFlag
{
	uint8_t b0: 1{};
	uint8_t b1: 1{};
	uint8_t b2: 1{};
	uint8_t b3: 1{};
	uint8_t b4: 1{};
	uint8_t b5: 1{};
	uint8_t b6: 1{};
	uint8_t b7: 1{};
public:
	explicit BitFlag(uint8_t init_byte)
	{
		setByte(init_byte = 0x00);
	}

	void setByte(uint8_t byte)
	{
		b0 = (byte >> 0) & 0x01;
		b1 = (byte >> 1) & 0x01;
		b2 = (byte >> 2) & 0x01;
		b3 = (byte >> 3) & 0x01;
		b4 = (byte >> 4) & 0x01;
		b5 = (byte >> 5) & 0x01;
		b6 = (byte >> 6) & 0x01;
		b7 = (byte >> 7) & 0x01;
	}

	[[nodiscard]] uint8_t toByte() const
	{
		return (b7 << 7) | (b6 << 6) | (b5 << 5) | (b4 << 4) | (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
	}

	void printBits() const
	{
		uint8_t bitset = toByte();
		for (int i = 7; i >= 0; --i)
		{
			std::cout << ((bitset >> i) & 0x01);
		}
		std::cout << std::endl;
	}
};

int main()
{
	BitFlag bitFlag{0xff};

	Time time = Time::morning;

	size_t size = sizeof(bitFlag);
	size = sizeof(time);

	Permission permission = Permission::Create |
							Permission::Update |
							Permission::Delete;
	size_t permissionSize = sizeof(permission);
	std::cout << static_cast<int>(permission) << std::endl;

	BitFlag bf{0x00};
	bf.setByte(0b10010010);// b7 -> b0
	bf.printBits();// bit-level display

	return 0;
}