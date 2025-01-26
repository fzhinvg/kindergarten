//
// Created by fzhinvg on 2025/1/26.
//
#include <windows.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <thread>

// 赛博光电开关

HBITMAP CaptureScreenArea(int x, int y, int width, int height)
{
	HDC hScreenDC = GetDC(nullptr);
	if (!hScreenDC)
	{
		std::cerr << "Failed to get screen device context!" << std::endl;
		return nullptr;
	}
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	if (!hMemoryDC)
	{
		std::cerr << "Failed to create memory device context!" << std::endl;
		ReleaseDC(nullptr, hScreenDC);
		return nullptr;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	if (!hBitmap)
	{
		std::cerr << "Failed to create compatible bitmap!" << std::endl;
		DeleteDC(hMemoryDC);
		ReleaseDC(nullptr, hScreenDC);
		return nullptr;
	}
	auto hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);
	if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, x, y, SRCCOPY))
	{
		std::cerr << "Failed to copy screen content!" << std::endl;
		SelectObject(hMemoryDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemoryDC);
		ReleaseDC(nullptr, hScreenDC);
		return nullptr;
	}
	SelectObject(hMemoryDC, hOldBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(nullptr, hScreenDC);
	return hBitmap;
}

SDL_Surface *CreateSDLSurfaceFromHBITMAP(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	int bitsPerPixel = bmp.bmBitsPixel;

	BITMAPINFOHEADER bmpInfoHeader;
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = bmp.bmWidth;
	bmpInfoHeader.biHeight = -bmp.bmHeight;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = bitsPerPixel;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;
	bmpInfoHeader.biXPelsPerMeter = 0;
	bmpInfoHeader.biYPelsPerMeter = 0;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;

	int bmpDataSize = ((bmp.bmWidth * bitsPerPixel + 31) / 32) * 4 * bmp.bmHeight;
	char *bmpData = new char[bmpDataSize];
	HDC hdc = GetDC(nullptr);
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, bmpData, (BITMAPINFO *) &bmpInfoHeader, DIB_RGB_COLORS);
	ReleaseDC(nullptr, hdc);

	// 为数据创建 SDL_Surface
	SDL_Surface *surface = SDL_CreateSurfaceFrom(bmp.bmWidth,
												 bmp.bmHeight,
												 SDL_PIXELFORMAT_ARGB8888,
												 (void *) bmpData,
												 bmp.bmWidthBytes);


	return surface ? surface : nullptr;
}

double CalculateAverageLuminance(SDL_Surface *surface)
{
	auto *pixels = (Uint32 *) surface->pixels;
	int pitch = surface->pitch / 4; // 计算每行像素数
	double totalLuminance = 0;
	int pixelCount = surface->w * surface->h;

	for (int y = 0; y < surface->h; ++y)
	{
		for (int x = 0; x < surface->w; ++x)
		{
			Uint32 pixel = pixels[y * pitch + x];
			Uint8 r, g, b;
			SDL_GetRGB(pixel,
					   SDL_GetPixelFormatDetails(surface->format),
					   nullptr,
					   &r,
					   &g,
					   &b);
			double luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;
			totalLuminance += luminance;
		}
	}

	double averageLuminance = totalLuminance / pixelCount;
	return averageLuminance;
}

#pragma region simulateKeyboard
HHOOK hKeyboardHook;

void simulateKeyPress(WORD vk)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(160));

	INPUT inputDown = {0};
	inputDown.type = INPUT_KEYBOARD;
	inputDown.ki.wVk = vk;
	inputDown.ki.dwFlags = 0; // 按下按键
	SendInput(1, &inputDown, sizeof(INPUT));

	INPUT inputUp = {0};
	inputUp.type = INPUT_KEYBOARD;
	inputUp.ki.wVk = vk;
	inputUp.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &inputUp, sizeof(INPUT));
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
	{
		auto *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
		if (pKeyboard->vkCode == VK_END)
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

	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

#pragma endregion

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Screen Capture",
										  256, 256,
										  SDL_WINDOW_ALWAYS_ON_TOP
	);
	if (!window)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	bool quit = false;
	SDL_Event e;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	HINSTANCE hInstance = GetModuleHandle(nullptr);
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

	double averageLuminance;

	std::thread task{[&averageLuminance]
					 {
						 for (;;)
						 {
							 if (averageLuminance > 9)
							 {
								 simulateKeyPress(VkKeyScan(' '));
							 }
						 }
					 }};
	task.detach();

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				quit = true;
			}
		}

		HBITMAP hBitmap = CaptureScreenArea(screenWidth / 2,
											screenHeight / 2,
											44,
											44);  // 捕捉屏幕区域

		if (hBitmap)
		{
			SDL_Surface *surface = CreateSDLSurfaceFromHBITMAP(hBitmap);  // 抓取内容转换为 SDL_Surface
			averageLuminance = CalculateAverageLuminance(surface);
			std::cout << averageLuminance << std::endl;

			if (surface)
			{
				SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
				if (texture)
				{
					SDL_RenderClear(renderer);
					SDL_RenderTexture(renderer, texture, nullptr, nullptr);
					SDL_RenderPresent(renderer);
					SDL_DestroyTexture(texture);
				}
				free(surface->pixels); // 内存溢出
				SDL_DestroySurface(surface);
			}
			DeleteObject(hBitmap);
		}

//		SDL_Delay(10);  // 延迟以减少 CPU 占用,即使不设置好像也并不怎么占用
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	UnhookWindowsHookEx(hKeyboardHook);
	return 0;
}
