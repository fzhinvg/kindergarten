#include "gameSave.h"

int main()
{
	GameSaveData saveData;
	saveData.level = 3;
	saveData.score = 1500;
	saveData.inventory = {1, 2, 3, 4, 5, 1, 23, 4, 5, 6, 7};
	saveData.timestamp = std::time(nullptr); // 获取当前时间戳

	try
	{
		saveData.serialize("gameSave.dat");
	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	GameSaveData loadedData;
	try
	{
		loadedData.deserialize("gameSave.dat");
		loadedData.display();
	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
