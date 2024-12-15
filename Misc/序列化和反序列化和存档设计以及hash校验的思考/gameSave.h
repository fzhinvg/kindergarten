#ifndef KINDERGARTEN_LOCAL_GAMESAVE_H
#define KINDERGARTEN_LOCAL_GAMESAVE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>//比chrono用起来无脑
#include <functional>
#include <stdexcept>
#include <limits>

/*
 * serialize 将对象的状态转换为可以存储或传输的格式的过程
 * deserialize 将字节流转换回对象的状态的过程
 * 说白了就是 说白了就是对象内存态到字节流存储态的互演
 * 一般常见的就是json和xml...
 * */

// 0x9e3779b9,换言之,使用常见的magic_number会增加被破解的风险.
constexpr size_t MAGIC = 0x1ead5417;

class GameSaveData
{
public:
	int level;
	int score;
	std::vector<int> inventory;
	time_t timestamp;

	void serialize(const std::string &filename) const;

	void deserialize(const std::string &filename);

	void display() const;
};

// 特化 std::hash<GameSaveData> 用于检验文件是否被修改
namespace std
{
	template<>
	struct hash<GameSaveData>
	{
		size_t operator()(const GameSaveData &data) const;
	};
}


#endif //KINDERGARTEN_LOCAL_GAMESAVE_H
