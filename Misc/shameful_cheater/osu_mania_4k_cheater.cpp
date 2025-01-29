//
// Created by fzhinvg on 2025/1/28.
// 请不要使用此代码以及生成的可执行文件,这是一个作弊程序
/*
 * 免责声明：
 * 本程序仅供学习和研究用途，严禁用于任何形式的作弊行为。
 * 严禁将本程序用于任何形式的商业行为。
 * 严禁以任何非法目的或途径使用本程序。
 * 作者不对因使用本程序而产生的任何直接或间接损失负责。
 * 作者保留程序的版权和知识产权，禁止任何未经授权的修改和再分发行为。
 * 使用者需自行承担使用本程序或由此产生的所有后果和责任。
 * 请尊重游戏规则，公平竞争。
 * 使用本源码以及生成的可执行程序默认已阅读此声明。
 *
 * Disclaimer:
 * This program is for educational and research purposes only.
 * The author is not responsible for any direct or indirect damage caused by using this program.
 * Commercial use of this program in any form is strictly prohibited.
 * The use of this program for any illegal purpose or by any illegal means is strictly prohibited.
 * The author retains copyright and intellectual property rights of the program. Unauthorized modification and redistribution are prohibited.
 * Users are responsible for any outcomes and liabilities resulting from the use of this code or the resulting executable program.
 * Please respect the game rules and engage in fair play.
 * By using this source code and any resulting executable program, you acknowledge that you have read, understood, and agreed to this disclaimer.
 */

/*
 * 程序逻辑:
 * 将对应osu文件重命名为target.osu放在同一个目录下
 * 运行后等待提示,使用 C 键目押第一个note,观察散布,如果不满意就使用按esc终止进程,重新运行.
 * */
#include <iostream>
#include <cstdint>
#include <vector>
#include <fstream> // handle .osu file
#include <sstream> // handle .osu file
#include <windows.h> // simulateKeyPress
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
//#include <iterator> // used in old version thread function

class KeyTrack; // 因为我不想分离文件所以需要前置声明
#pragma region debugTool

class TimelineTracker
{
public:
	TimelineTracker() : start_time_point(std::chrono::high_resolution_clock::now())
	{}

	// 启动计时器
	void start()
	{
		start_time_point = std::chrono::high_resolution_clock::now();
	}

	// 获取已经流逝的时间，单位为毫秒
	[[nodiscard]] long long getElapsedTime() const
	{
		auto end_time_point = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_point - start_time_point).count();
		return elapsed;
	}

private:
	std::chrono::high_resolution_clock::time_point start_time_point;
};

#pragma endregion

#pragma region simulateKeyPress

void simulateKeyPress(WORD vk, time_t holding_time = 1) // time(ms)
{
	static thread_local INPUT inputDown = {[&]
										   {
											   INPUT temp = {0};
											   temp.type = INPUT_KEYBOARD;
											   temp.ki.wVk = vk;
											   temp.ki.dwFlags = 0; // 按下按键
											   return temp;
										   }()}; // 使用 thread_local
	static thread_local INPUT inputUp = {[&]
										 {
											 INPUT temp = {0};
											 temp.type = INPUT_KEYBOARD;
											 temp.ki.wVk = vk;
											 temp.ki.dwFlags = KEYEVENTF_KEYUP;
											 return temp;
										 }()}; // 每个线程都有自己独立的静态变量,不会与其他线程共享,避免数据竞争
//	std::cout << " down ";

	SendInput(1, &inputDown, sizeof(INPUT));

//	std::cout << "holding" << holding_time << " ";
	std::this_thread::sleep_for(std::chrono::milliseconds(holding_time));

	SendInput(1, &inputUp, sizeof(INPUT));
//	std::cout << " up" << std::endl;

}

void resetKeyTrack()
{
	std::thread reset_d{[]
						{
							simulateKeyPress(VkKeyScan('d'));
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}};
	std::thread reset_f{[]
						{
							simulateKeyPress(VkKeyScan('f'));
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}};
	std::thread reset_j{[]
						{
							simulateKeyPress(VkKeyScan('j'));
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}};
	std::thread reset_k{[]
						{
							simulateKeyPress(VkKeyScan('k'));
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}};
	reset_d.join();
	reset_f.join();
	reset_j.join();
	reset_k.join();
}

#pragma endregion

#pragma region osuFileHandle

struct HitObject
{
	int x;
	int y;
	time_t click_time;
	int key_type; // note and long note
	int sound_effect; // useless in our project
	time_t end_time; // long note's end click_time

	HitObject() : x(0),
				  y(0),
				  click_time(0),
				  key_type(0),
				  sound_effect(0),
				  end_time(0)
	{}

	HitObject(int x_, int y_, time_t time_, int key_type_, int sound_effect_, time_t end_time_) :
			x(x_),
			y(y_),
			click_time(time_),
			key_type(key_type_),
			sound_effect(sound_effect_),
			end_time(end_time_)
	{}
};

void parseOsuFile(const std::string &filePath, std::vector<HitObject> &note_vector)
{
	std::ifstream file(filePath);
	std::string line;
	bool inHitObjectsSection = false;

	while (std::getline(file, line))
	{
		if (line == "[HitObjects]")
		{
			inHitObjectsSection = true;
			continue;
		}

		if (inHitObjectsSection)
		{
			std::istringstream ss(line);
			HitObject obj;
			char delim;
			if (ss >> obj.x >> delim >> obj.y >> delim >> obj.click_time >> delim >> obj.key_type >> delim
				   >> obj.sound_effect >> delim >> obj.end_time)
			{
				note_vector.push_back(obj);  // 填充 vector
			}
		}
	}
}

#pragma endregion

#pragma region key.cpp
// 完全可以提取到一个cpp文件中,但是没啥必要在这里

enum KeyType
{
	note = 1,
	long_note = 128
};
enum KeyValue
{
	d = 64,
	f = 192,
	j = 320,
	k = 448
};
time_t forward_offset = 0; // 用于目押

class Key
{
private:
	KeyValue _key_value;
	KeyType _key_type;
	time_t _click_time;
	time_t _end_time;
	time_t _holding_time;
public:
	explicit Key(const HitObject &note) : _click_time(note.click_time - forward_offset), _end_time(note.end_time - forward_offset)
	{
		switch (note.x)
		{
			case KeyValue::d :
				this->_key_value = KeyValue::d;
				break;
			case KeyValue::f :
				this->_key_value = KeyValue::f;
				break;
			case KeyValue::j :
				this->_key_value = KeyValue::j;
				break;
			case KeyValue::k :
				this->_key_value = KeyValue::k;
				break;
			default:
				std::cerr << "wrong key value" << note.x << std::endl;
				break;
		}
		switch (note.key_type)
		{
			case KeyType::note:
				this->_key_type = KeyType::note;
				break;
			case KeyType::long_note:
				this->_key_type = KeyType::long_note;
				break;
			default:
//				std::cerr << "wrong key type" << note.key_type << std::endl;
				this->_key_type = KeyType::note;
				break;
		}
		if (this->_key_type == KeyType::long_note)
			this->_holding_time = this->_end_time - this->_click_time;
	}

	void display() // test only
	{
		if (this->_key_type == KeyType::note)
			std::cout << "note: " << this->_key_value << " " << this->_click_time << '\n';
		else if (this->_key_type == KeyType::long_note)
			std::cout << "long note: " << this->_key_value << " " << this->_click_time << " holding "
					  << this->_holding_time << '\n';
	}

	friend KeyTrack;

	friend void separate2tracks(std::vector<Key> &key_vector,
								KeyTrack &d_track,
								KeyTrack &f_track,
								KeyTrack &j_track,
								KeyTrack &k_track);
};

#pragma endregion

#pragma region KeyTrackAndRunningThread

//std::mutex mtx;
//std::condition_variable cv;
std::atomic<bool> runningSignal(true);
bool ready = false; // 用于对齐4个轨道启动时间

class KeyTrack
{
private:
	KeyValue _track_key_value;
	std::vector<Key> key_vector;
public:
	explicit KeyTrack(KeyValue keyValue) : _track_key_value(keyValue)
	{}

	void add_key(Key &key)
	{
		if (key._key_value == this->_track_key_value)
		{
			key_vector.emplace_back(key);
		}
	}

	size_t get_key_vector_size() // test only
	{
		return key_vector.size();
	}

	void display() // only used in test
	{
		for (auto &key: key_vector)
		{
			key.display();
		}
	}

	void RunTrack() // thread function polling version
	{
//		time_t pre_note_click_time = 0;
		char vk_char;
		switch (_track_key_value)
		{
			case KeyValue::d:
				vk_char = 'd';
				break;
			case KeyValue::f:
				vk_char = 'f';
				break;
			case KeyValue::j:
				vk_char = 'j';
				break;
			case KeyValue::k:
				vk_char = 'k';
				break;
			default:
				std::cerr << "fatal error" << std::endl;
				break;
		}
		auto vk = VkKeyScan(vk_char);
		TimelineTracker timelineTracker{};
		time_t predict_offset = 5;
		auto top = key_vector.begin();

//		std::unique_lock<std::mutex> lck(mtx);
//		cv.wait(lck, []
//		{ return ready; });
//		lck.unlock();
//		cv.notify_one();

		while (!ready)
		{
			// 放弃使用锁,提高cpu占用,但是可以提高线程同步精度
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

		std::cout << std::chrono::system_clock::now() << std::endl;
		timelineTracker.start();
		while (top != key_vector.end() && runningSignal)
		{
			if (timelineTracker.getElapsedTime() >= (top->_click_time - predict_offset))
			{
				if (top->_key_type == KeyType::note)
				{
//					std::cout << timelineTracker.getElapsedTime();
					simulateKeyPress(vk);
					++top;
				} else
				{
//					std::cout << timelineTracker.getElapsedTime();
					simulateKeyPress(vk, top->_holding_time);
					++top;
				}
			}
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

	}

//	void old_version_RunTrack() // thread function
//	{
//		std::unique_lock<std::mutex> lck(mtx);
//		cv.wait(lck, []
//		{ return ready; });
//		lck.unlock();
//		cv.notify_all();
//		std::cout << "Task:" << this->_track_key_value << " is running" << std::endl;
//		time_t pre_note_click_time = 0;
//		char vk_char;
//		switch (_track_key_value)
//		{
//			case KeyValue::d:
//				vk_char = 'd';
//				break;
//			case KeyValue::f:
//				vk_char = 'f';
//				break;
//			case KeyValue::j:
//				vk_char = 'j';
//				break;
//			case KeyValue::k:
//				vk_char = 'k';
//				break;
//			default:
//				std::cerr << "fatal error" << std::endl;
//				break;
//		}
//
//		TimelineTracker timelineTracker{};
//		timelineTracker.start();
//		for (const auto &key: this->key_vector)
//		{
//			if (key._key_type == KeyType::note)
//			{
//				std::this_thread::sleep_for(std::chrono::milliseconds(key._click_time - pre_note_click_time));
//				std::cout << timelineTracker.getElapsedTime() << " ";
//				simulateKeyPress(VkKeyScan(vk_char));
//				pre_note_click_time = key._click_time;
//			} else
//			{
//				std::this_thread::sleep_for(
//						std::chrono::milliseconds(key._click_time - pre_note_click_time - key._holding_time));
//				std::cout << timelineTracker.getElapsedTime() << " ";
//				simulateKeyPress(VkKeyScan(vk_char), key._holding_time);
//				pre_note_click_time = key._click_time;
//			}
//
//		}
//
//	}
};

void separate2tracks(std::vector<Key> &key_vector,
					 KeyTrack &d_track,
					 KeyTrack &f_track,
					 KeyTrack &j_track,
					 KeyTrack &k_track)
{
	for (auto &key: key_vector)
	{
		switch (key._key_value)
		{
			case KeyValue::d:
				d_track.add_key(key);
				break;
			case KeyValue::f:
				f_track.add_key(key);
				break;
			case KeyValue::j:
				j_track.add_key(key);
				break;
			case KeyValue::k:
				k_track.add_key(key);
				break;
			default:
				key.display();
				std::cerr << "cannot add to track" << std::endl;
				break;
		}
	}

}

#pragma endregion

#pragma region GlobalMonitoring

HHOOK hKeyboardHook;
//HHOOK hMouseHook;
//time_t wait = 100'000; // 搞了半天不如手动目押


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool first_time = true;
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
	{
		auto *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
		if (pKeyboard->vkCode == VK_ESCAPE)
		{
//			UnhookWindowsHookEx(hMouseHook);
			runningSignal.store(false);
			UnhookWindowsHookEx(hKeyboardHook);
			resetKeyTrack();
			std::terminate();
		} else if (pKeyboard->vkCode == 'C') // 搞了半天各种对齐方式,到最后还不如目押
		{
			if (first_time)
			{
				std::cout << "all start!!! " << std::endl;
//				std::this_thread::sleep_for(std::chrono::microseconds(wait));
//			std::lock_guard<std::mutex> lck(mtx);
				ready = true;
//			cv.notify_all(); // 通知key_track.RunTrack()
				first_time = false;
			}
		}
	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

//LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//	if (nCode == HC_ACTION && wParam == WM_LBUTTONDOWN)
//	{
//
//	}
//	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
//}

#pragma endregion

int main()
{
	// 获取当前进程的句柄
	HANDLE hProcess = GetCurrentProcess();

	// 设置进程优先级为 HIGH_PRIORITY_CLASS
	if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
	{
		std::cout << "Process priority set to HIGH_PRIORITY_CLASS." << std::endl;
	} else
	{
		std::cerr << "Failed to set process priority." << std::endl;
	}

	HINSTANCE hInstance = GetModuleHandle(nullptr);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
	if (!hKeyboardHook)
	{
		std::cerr << "Failed to install keyboard hook." << std::endl;
		return -1;
	}
//	hKeyboardHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);


	// 解析所有的note
	std::vector<HitObject> note_vector;
	parseOsuFile("target.osu", note_vector);

	forward_offset=(note_vector.begin()->click_time-10); // 用于目押

	// 把所有的note转换成我自己的key结构
	std::vector<Key> key_vector;
	key_vector.reserve(note_vector.size());
	for (const auto &obj: note_vector)
	{
		key_vector.emplace_back(obj);
	}

	// 分轨 4k -> df_jk
	KeyTrack d_track{KeyValue::d};
	KeyTrack f_track{KeyValue::f};
	KeyTrack j_track{KeyValue::j};
	KeyTrack k_track{KeyValue::k};

	separate2tracks(key_vector, d_track, f_track, j_track, k_track);

	// test only
//	f_track.display();

	size_t key_cout = d_track.get_key_vector_size() +
					  f_track.get_key_vector_size() +
					  j_track.get_key_vector_size() +
					  k_track.get_key_vector_size();

	std::cout << key_cout << std::endl;

	std::thread d_task{[&]
					   {
						   d_track.RunTrack();
					   }};
	std::thread f_task{[&]
					   {
						   f_track.RunTrack();
					   }};
	std::thread j_task{[&]
					   {
						   j_track.RunTrack();
					   }};
	std::thread k_task{[&]
					   {
						   k_track.RunTrack();
					   }};


	d_task.detach();
	f_task.detach();
	j_task.detach();
	k_task.detach();
	std::cout << "get ready..." << std::endl;

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	UnhookWindowsHookEx(hKeyboardHook);
	return 0;
}