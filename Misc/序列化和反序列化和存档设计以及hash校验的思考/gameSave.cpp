#include "gameSave.h"

void GameSaveData::serialize(const std::string &filename) const
{
	std::ofstream outFile(filename, std::ios::binary);

	if (!outFile.is_open())
	{
		std::cerr << "Error opening file for saving." << std::endl;
		return;
	}

	// 计算本身hash,第一个推入文件流是为了读取的时候不需要再次移动指针到对应位置
	// 感觉这种自定义二进制文件很大的一个难点就是数据布局和指针管理
	std::hash<GameSaveData> hash_maker;
	size_t hash_value = hash_maker(*this);
	outFile.write(reinterpret_cast<const char *>(&hash_value), sizeof(hash_value));

	outFile.write(reinterpret_cast<const char *>(&level), sizeof(level));
	outFile.write(reinterpret_cast<const char *>(&score), sizeof(score));

	size_t inventorySize = inventory.size();
	// size 和 data 需要分开存放,是因为自定义二进制文件并不知道vector等各种容器的结构,在反序列化的时候需要知道size来重新构造对应容器
	// 写到这里,我突然就理解了json的好,但是使用自定义二进制文件,这很酷,不是吗?
	if (inventorySize >
		static_cast<size_t>(std::numeric_limits<std::streamsize>::max()))
	{
		throw std::overflow_error("Inventory size exceeds std::streamsize limit.");
	}// 我决定就此打住，不再深究，毕竟只是个玩具代码，我想不应该花费精力在并不会使用的东西上
	// 但是话又说回来，玩具代码就是不会使用的东西，所以我还是要深究怎么解决narrowing conversion
	auto streamInventorySize = static_cast<std::streamsize>(inventorySize);
	outFile.write(reinterpret_cast<const char *>(&streamInventorySize), sizeof(streamInventorySize));
	// 这里大部分情况下是没有问题的，因为一个存档文件不太可能出现大小大于 long long 导致转为一个负数
	// 所以这里的检查可能有点多余，完全可以删除上面的throw
	outFile.write(reinterpret_cast<const char *>(inventory.data()), streamInventorySize * sizeof(int));
	outFile.write(reinterpret_cast<const char *>(&timestamp), sizeof(timestamp));

	outFile.close();
	std::cout << "Game saved successfully." << std::endl;

}

void GameSaveData::deserialize(const std::string &filename)
{
	std::ifstream inFile(filename, std::ios::binary);
	if (!inFile.is_open())
	{
		std::cerr << "Error opening file for loading." << std::endl;
		return;
	}

	// 读取并校验哈希值
	size_t saved_hash_value;
	inFile.read(reinterpret_cast<char *>(&saved_hash_value), sizeof(saved_hash_value));

	// 暂时读取其他数据到临时变量
	GameSaveData tempData;// 用一个中间临时变量实现swap简单而有效,这里用于临时存储this,待hash校验后重新赋值
	inFile.read(reinterpret_cast<char *>(&tempData.level), sizeof(tempData.level));
	inFile.read(reinterpret_cast<char *>(&tempData.score), sizeof(tempData.score));

	std::streamsize streamInventorySize;
	inFile.read(reinterpret_cast<char *>(&streamInventorySize), sizeof(streamInventorySize));
	if (static_cast<size_t>(streamInventorySize) > tempData.inventory.max_size())
	{
		throw std::overflow_error("Stream inventory size is invalid.");
	}
	tempData.inventory.resize(static_cast<size_t>(streamInventorySize));
	inFile.read(reinterpret_cast<char *>(tempData.inventory.data()), streamInventorySize * sizeof(int));
	inFile.read(reinterpret_cast<char *>(&tempData.timestamp), sizeof(tempData.timestamp));

	// 计算当前对象的哈希值并比对
	std::hash<GameSaveData> hash_maker;
	size_t current_hash_value = hash_maker(tempData);
	if (current_hash_value != saved_hash_value)
	{
		throw std::runtime_error("Save file is corrupted!");
	}

	// 如果校验通过，将临时变量的数据赋值给当前对象
	*this = tempData;

	inFile.close();
	std::cout << "Save loaded successfully." << std::endl;
}


void GameSaveData::display() const
{
	std::cout << "Level: " << level << "\nScore: " << score << "\nInventory: ";
	for (int item: inventory)
	{
		std::cout << item << " ";
	}
	std::cout << "\nLast Save Time: " << std::ctime(&timestamp);
}

// 特化 std::hash<GameSaveData> 的定义
namespace std
{
	size_t hash<GameSaveData>::operator()(const GameSaveData &data) const
	{
		size_t hash_value = 0;
		// 同理使用左移6 右移2 也有风险,但是提供了更好的散度,打得更散
		hash_value ^= std::hash<int>{}(data.level) + MAGIC + (hash_value << 6) + (hash_value >> 2);
		hash_value ^= std::hash<int>{}(data.score) + MAGIC + (hash_value << 1) + (hash_value >> 2);
		for (int item: data.inventory)
		{
			hash_value ^= std::hash<int>{}(item) + MAGIC + (hash_value << 3) + (hash_value >> 4);
		}
		hash_value ^= std::hash<time_t>{}(data.timestamp) + MAGIC + (hash_value << 5) + (hash_value >> 6);
		return hash_value;
	}
}

#if 0
// 简洁,但容易受到内存布局变化的影响,可能会导致哈希值不一致
// 有点偏向于编译器,系统相关了,就像是吧windows上生成的文件拿到macos上就无法解析了一样
// 感觉不是很妙
namespace std {
	template<>
	struct hash<GameSaveData> {
		size_t operator()(const GameSaveData& data) const {
			return std::hash<std::string>{}(std::string(reinterpret_cast<const char*>(&data), sizeof(data)));
		}
	};
}
// 至于为什么不使用md5之类的,想必应该没有这么自相矛盾的问题
// 虽然我感觉这一切在逆向面前都没什么加密性可言
// 但是,哈希函数的主要目的是保证数据的完整性和唯一性,而不是提供真正的加密保护.
// 但是怎么用那就是自己说了算了
#endif