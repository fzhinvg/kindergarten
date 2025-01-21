//
// Created by fzhinvg on 2025/1/21.
//
#include <iostream>
#include <windows.h>
#include <thread>
#include <random>
//#include <chrono>

// 0.声明一个钩子
HHOOK hKeyboardHook;

// 此文件是第一次引入.ps1进行手动编译
// 启用CMakeLists中相关编译选项以关闭控制台

// 这个函数是用来生成随机移动的方向
int generateRandomNumber()
{
	// 创建随机数引擎
	static std::random_device rd;  // 随机数种子
	static std::mt19937 gen(rd()); // 标准梅森旋转算法
	static std::uniform_int_distribution<> dis(0, 3); // 定义随机数分布范围
	// 这里是将0123当作x,y轴方向使用的,分别对应上下左右,所以:
	// 0 -> y-- 上
	// 1 -> y++ 下
	// 2 -> x-- 左
	// 3 -> x++ 右
	return dis(gen);
}

// 这个函数是随机生成屏幕上的点,用于移动鼠标
POINT generateRandomPoint()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	static std::random_device rd;  // 随机数种子
	static std::mt19937 gen(rd()); // 标准梅森旋转算法
	static std::uniform_int_distribution<> xDis(0, screenWidth);
	static std::uniform_int_distribution<> yDis(0, screenHeight);

	return POINT(xDis(gen), yDis(gen));
}

// 这个函数用于在客户区轮询检查客户区范围
void cursorPollingJump(HWND hwnd)
{
	std::cout << __func__ << std::endl;
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	const uint32_t horizontalJumpCnt = width / 64;
	const uint32_t verticalJumpCnt = height / 64;
	POINT cursorTemp;
	GetCursorPos(&cursorTemp);

	POINT cursorPos(12, 12);
	// 屏幕坐标转换到客户区坐标
//	ScreenToClient(hwnd, &cursorPos); wtf?
	ClientToScreen(hwnd, &cursorPos);

	for (int vCnt = 0; vCnt < verticalJumpCnt; ++vCnt)
	{
		for (int hCnt = 0; hCnt < horizontalJumpCnt; ++hCnt)
		{
			SetCursorPos(cursorPos.x + (hCnt * 64), cursorPos.y);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		cursorPos.y += 64;
	}

	// 恢复鼠标初始位置
	SetCursorPos(cursorTemp.x, cursorTemp.y);
}

// 字面意思,仅作测试
[[noreturn]] void parkinson()
{
	std::cout << __func__ << std::endl;
	POINT nowPos;
	while (true)
	{
		GetCursorPos(&nowPos);
		switch (generateRandomNumber())
		{
			case 0:
				SetCursorPos(nowPos.x, nowPos.y - 1);
				break;
			case 1:
				SetCursorPos(nowPos.x, nowPos.y + 1);
				break;
			case 2:
				SetCursorPos(nowPos.x - 1, nowPos.y);
				break;
			case 3:
				SetCursorPos(nowPos.x + 1, nowPos.y);
				break;
			default:
				break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

// 字面意思,仅作测试
[[noreturn]] void AreYouInsane()
{
	std::cout << __func__ << std::endl;
	while (true)
	{
		POINT point = generateRandomPoint();
		SetCursorPos(point.x, point.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

// 为之后可能的软件宏做可行性测试
void simulateMouseClick(int x, int y)
{
	std::cout << __func__ << std::endl;
	// 移动鼠标
	SetCursorPos(x, y);

	// 按下左键
	INPUT inputDown = {0};
	inputDown.type = INPUT_MOUSE;
	inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &inputDown, sizeof(INPUT));

	// 模拟自然延迟 应该加入一点波动,可以把这个作为一个带初始值的参数
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// 松开左键
	INPUT inputUp = {0};
	inputUp.type = INPUT_MOUSE;
	inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &inputUp, sizeof(INPUT));

	// 逻辑上点击鼠标这个动作是有后摇的
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// 经测试windows默认click事件的判断是按下触发,只有抬起是不触发的,但是这个具体得看各个application的按钮定义
}

void simulateKeyPress(WORD vk)
{
	std::cout << __func__ << std::endl;
	// 按下按键
	INPUT inputDown = {0};
	inputDown.type = INPUT_KEYBOARD;
	inputDown.ki.wVk = vk;
	inputDown.ki.dwFlags = 0; // 按下按键
	SendInput(1, &inputDown, sizeof(INPUT));

	// 模拟自然延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	// 松开按键
	INPUT inputUp = {0};
	inputUp.type = INPUT_KEYBOARD;
	inputUp.ki.wVk = vk;
	inputUp.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &inputUp, sizeof(INPUT));

	// 后摇
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void simulateCombinationKeyPress(WORD vk1, WORD vk2, int delay = 100)
{
	// 按下第一个按键（例如 Ctrl）
	INPUT inputDown1 = {0};
	inputDown1.type = INPUT_KEYBOARD;
	inputDown1.ki.wVk = vk1;
	inputDown1.ki.dwFlags = 0; // 按下按键
	SendInput(1, &inputDown1, sizeof(INPUT));

	// 模拟自然延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));

	// 按下第二个按键（例如 C）
	INPUT inputDown2 = {0};
	inputDown2.type = INPUT_KEYBOARD;
	inputDown2.ki.wVk = vk2;
	inputDown2.ki.dwFlags = 0; // 按下按键
	SendInput(1, &inputDown2, sizeof(INPUT));

	// 模拟自然延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));

	// 松开第二个按键（例如 C）
	INPUT inputUp2 = {0};
	inputUp2.type = INPUT_KEYBOARD;
	inputUp2.ki.wVk = vk2;
	inputUp2.ki.dwFlags = KEYEVENTF_KEYUP; // 松开按键
	SendInput(1, &inputUp2, sizeof(INPUT));

	// 模拟自然延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));

	// 松开第一个按键（例如 Ctrl）
	INPUT inputUp1 = {0};
	inputUp1.type = INPUT_KEYBOARD;
	inputUp1.ki.wVk = vk1;
	inputUp1.ki.dwFlags = KEYEVENTF_KEYUP; // 松开按键
	SendInput(1, &inputUp1, sizeof(INPUT));

	// 后续的延迟
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

// 现在我需要验证最后一步,全局监听,在我切出窗口之后也可以接收到宏开启的按键输入信号

// 回调函数：窗口过程
// LRESULT long integer 用于返回消息处理的结果
// CALLBACK 一个宏,定义了调用约定,表示该函数是一个回调函数
// HWND hwnd 窗口句柄handle,函数可以使用这个句柄来引用和操作窗口.获取窗口属性,发送消息... // HWND -> Handle to a Window
// UINT uMsg unsigned integer 表示消息的类型.指示窗口过程需要处理的消息类型
// WPARAM wParam unsigned integer-sized type 用于传递与消息相关的附加信息,具体含义取决于消息类型.
// 例如,对于按键消息 WM_KEYDOWN ,wParam 表示按键的虚拟键码
// LPARAM lParam long integer-sized type 用于传递与消息相关的附加信息
// 例如,对于鼠标点击消息 WM_LBUTTONDOWN,lParam 表示鼠标点击的位置坐标
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const std::string messagePrinted = "Do not press the \'I\' on your keyboard!!!";
	static HFONT hFont = nullptr; // 字体句柄
	switch (uMsg)
	{
		case WM_CREATE:
		{
			// 创建字体
			hFont = CreateFont(
					24,                        // 字体高度
					0,                         // 字体宽度
					0,                         // 文字倾斜角度
					0,                         // 基准线倾斜角度
					FW_LIGHT,                   // 字体粗细
					FALSE,                     // 是否斜体
					FALSE,                     // 是否下划线
					FALSE,                     // 是否删除线
					ANSI_CHARSET,              // 字符集
					OUT_TT_PRECIS,             // 输出精度
					CLIP_DEFAULT_PRECIS,       // 裁剪精度
					DEFAULT_QUALITY,           // 输出质量
					DEFAULT_PITCH | FF_SWISS,  // 字体系列
					TEXT("Consolas")              // 字体名称
			);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			RECT clientRect;
			GetClientRect(hwnd, &clientRect);


			// 设置文本颜色和背景颜色
			SetTextColor(hdc, RGB(255, 0, 0));
			SetBkMode(hdc, TRANSPARENT);

			// 选择字体
			auto hOldFont = (HFONT) SelectObject(hdc, hFont);

			// 绘制文本
			DrawText(hdc,
					 messagePrinted.c_str(),
					 -1,
					 &clientRect,
					 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			// 恢复旧字体
			SelectObject(hdc, hOldFont);

			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_KEYDOWN:
		{ //我发现这个东西的一个弊端,他好像无法同时对多个输入状态进行判断
			std::cout << "Key pressed: " << wParam << std::endl;
			if (GetAsyncKeyState(VK_F1) & 0x8000)
			{
				// 应该新建一个线程detach掉来执行动作,防止阻塞主线程;
				// 之后如果要做成页面宏,就应该整一个动作序列的功能!不然无法实现宏这么个功能
				// 话说为什么叫宏?这个翻译其实蛮怪的
				std::thread Task([]
								 {
									 for (int i = 0; i < 10; ++i)
									 {
										 simulateMouseClick(2047 / 2, 1151 / 2);
									 }
								 });
				Task.detach();
			}
			if (GetAsyncKeyState(VK_F2) & 0x8000)
			{
				// 此时我还没有设计全局监听,只能在当前窗口监听到之后才可以触发
				// 所以我这里先需要一个延时来让我移动光标到一个文本框
				std::this_thread::sleep_for(std::chrono::seconds(3));
				std::string inputString = "test text\n"; // 我的好奇心让我测试\n是不是等价于enter键,答案貌似是等价的
				std::thread Task([inputString]
								 {
									 for (auto &item: inputString)
									 {
										 simulateKeyPress(VkKeyScan(item));
									 }
								 });
				Task.detach();
			}
			if (wParam == 'W')
			{  // 'W' 键向上移动鼠标
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				SetCursorPos(cursorPos.x, cursorPos.y - 10);
			} else if (wParam == 'A')
			{  // 'A' 键向左移动鼠标
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				SetCursorPos(cursorPos.x - 10, cursorPos.y);
			} else if (wParam == 'S')
			{  // 'S' 键向下移动鼠标
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				SetCursorPos(cursorPos.x, cursorPos.y + 10);
			} else if (wParam == 'D')
			{  // 'D' 键向右移动鼠标
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				SetCursorPos(cursorPos.x + 10, cursorPos.y);
			} else if (wParam == 'J')
			{
				std::thread Task(cursorPollingJump, hwnd);
				Task.detach();
			} else if (wParam == 'P')
			{
				std::thread Task(parkinson);
				Task.detach(); // 这是我仅有的温柔
			} else if (wParam == 'I')
			{
				std::thread Task(AreYouInsane);
				Task.detach();
//				Task.join(); // 我们至少该做个人
			} else if ((wParam == 'C') && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) // ctrl+c to escape
			{
				PostQuitMessage(0);
				return 0; // 我最后的善良😇
			}
			break;
		}
		case WM_LBUTTONDOWN:
			std::cout << "Left mouse button clicked at (" << LOWORD(lParam) << ", " << HIWORD(lParam) << ")"
					  << std::endl;
			break;
		case WM_RBUTTONDOWN:
			std::cout << "Right mouse button clicked at (" << LOWORD(lParam) << ", " << HIWORD(lParam) << ")"
					  << std::endl;
			break;
		case WM_MOUSEMOVE:
//			std::cout << "Mouse moved to (" << LOWORD(lParam) << ", " << HIWORD(lParam) << ")" << std::endl;
			break;
		case WM_DESTROY:
			DeleteObject(hFont);  // 删除字体对象
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 1.钩子回调
// 钩子和窗口对于windows来说是两个东西,那些音乐播放器的全局热键功能就是钩子实现的
// nCode 一个整数代码,指示钩子的状态.HC_ACTION 表示钩子过程应该处理当前的消息
// wParam 消息参数,表示键盘消息的类型(例如 WM_KEYDOWN 表示按键按下事件)
// lParam 指向包含钩子信息的结构体的指针(例如 KBDLLHOOKSTRUCT)

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
	{
//		auto *pKeyboard = (KBDLLHOOKSTRUCT *) lParam;

		// pKeyboard: KBDLLHOOKSTRUCT 结构体,包含有关低级键盘输入事件的信息.
		// 该结构体包含以下字段:
		//vkCode: 虚拟键码,表示哪个键被按下或释放.
		//scanCode: 硬件扫描码,表示哪个物理键被按下或释放.
		//flags: 事件标志,指示是否按下了扩展键,上升键等.
		//time: 事件时间戳.
		//dwExtraInfo: 额外信息.

		auto *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
		if (pKeyboard->vkCode == VK_F3)
		{
			std::string inputString = "1qaz3wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-['=]\\`";
			std::thread Task([inputString]
							 {
								 for (auto &item: inputString)
								 {
									 simulateKeyPress(VkKeyScan(item));
								 }
							 });
			Task.detach();
		}
		if (pKeyboard->vkCode == VK_F4)
		{
			std::thread Task([]
							 {
								 simulateCombinationKeyPress(VK_CONTROL, VkKeyScanA('W'));
							 });
			Task.detach();
		}
	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}
// 至此,我已经完成了,构建一个软件宏软件的必要可行性测试
// 接下来我需要完成一下程序设计,以及采纳怎么样的前端设计

int main()
{
	std::cout << "Listening for key press..." << std::endl;

	// 2.安装全局键盘钩子
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

#pragma region windowProgress
	// ===---------------------------------------------=== 窗口过程
	// 1.注册窗口类
	const char CLASS_NAME[] = "Sample Window Class";
	// 窗口类名用于在注册和创建窗口时识别窗口的类别.每个窗口类都需要有一个唯一的类名,以便 Windows 可以正确地创建和管理窗口
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc; // 设置窗口过程回调函数 WindowProc,用于获取窗口消息
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	// 2.创建窗口
	HWND hwnd = CreateWindowEx(
			0,
			CLASS_NAME,
			"Display Window",
			WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
			CW_USEDEFAULT, CW_USEDEFAULT, 1024, 512,
			nullptr,
			nullptr,
			GetModuleHandle(nullptr),
			nullptr
	);

	if (hwnd == nullptr)
	{
		std::cerr << "Failed to create window." << std::endl;
		return 1;
	}

	// 3.显示窗口
	ShowWindow(hwnd, SW_NORMAL);

	// 4.运行消息循环
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//	// ===---------------------------------------------===
#pragma endregion

	// 3.卸载键盘钩子
	UnhookWindowsHookEx(hKeyboardHook);
	return 0;
}

#if consoleVer
#include <iostream>
#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>

// 函数：检测键盘按键状态
[[noreturn]] void checkKeyState() {
	while (true) {
		// 检查所有可能的按键状态
		for (int vkCode = 0x08; vkCode <= 0xFF; ++vkCode) {
			if (GetAsyncKeyState(vkCode) & 0x8000) {
				std::cout << "Key pressed: " << vkCode << std::endl;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

// 函数：检测鼠标点击
[[noreturn]] void checkMouseClick() {
	while (true) {
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
			std::cout << "Left mouse button clicked." << std::endl;
		}
		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
			std::cout << "Right mouse button clicked." << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

// 函数：打印鼠标位置
[[noreturn]] void printMousePosition() {
	POINT cursorPos;
	while (true) {
		if (GetCursorPos(&cursorPos)) {
			std::cout << "Mouse Position: (" << cursorPos.x << ", " << cursorPos.y << ")" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));  // 每1秒更新一次
	}
}

int main() {
	// 启动独立线程检测键盘按键状态
	std::thread keyboardThread(checkKeyState);
	// 启动独立线程检测鼠标点击
	std::thread mouseThread(checkMouseClick);
	// 启动独立线程打印鼠标位置
//	std::thread mousePositionThread(printMousePosition);

	// 等待用户按下 'Esc' 键退出
	while (true) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 检查按键间隔
	}

	// 终止线程
	keyboardThread.detach();       // 分离线程，以便在程序结束时自动终止
	mouseThread.detach();          // 分离线程，以便在程序结束时自动终止
//	mousePositionThread.detach();  // 分离线程，以便在程序结束时自动终止

	std::cout << "Program exited." << std::endl;
	return 0;
}
#endif

#if old_version
#include <iostream>
#include <windows.h>
#include <thread>

// 函数：获取并打印鼠标指针的位置
void printMousePosition()
{
	POINT cursorPos;
	while (true)
	{
		// 获取鼠标指针位置
		if (GetCursorPos(&cursorPos))
		{
			std::cout << "Mouse Position: (" << cursorPos.x << ", " << cursorPos.y << ")" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 每500毫秒更新一次
	}
}

void printScreenResolution()
{
	// 获取屏幕分辨率
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	std::cout << "Screen Resolution: " << screenWidth << "x" << screenHeight << std::endl;
}

int main()
{
	printScreenResolution();

	// 启动一个线程打印鼠标位置
	std::thread mouseThread(printMousePosition);

	// 等待用户按下 'Esc' 键退出
	while (true)
	{
		// 0b1000'0000'0000'0000
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 检查按键间隔
	}

	// 终止线程
	if (mouseThread.joinable())
	{
		mouseThread.detach();  // 分离线程，以便在程序结束时自动终止
	}

	std::cout << "Program exited." << std::endl;
	return 0;
}
#endif