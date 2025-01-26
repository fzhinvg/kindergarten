//
// Created by fzhinvg on 2025/1/25.
// 用于测试SDL和SDL_image是否正确链接
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

int main()
{
	// 初始化 SDL
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	// 创建窗口
	SDL_Window *window = SDL_CreateWindow("JPEG Display",
										  100,
										  100,
										  SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// 创建渲染器
	SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// 加载 JPEG 图片
	SDL_Surface *loadedSurface = IMG_Load("example.png"); // 确保路径正确，文件存在

	if (!loadedSurface)
	{
		std::cerr << "Unable to load image! SDL_image Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// 创建纹理
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	SDL_DestroySurface(loadedSurface); // 释放 surface
	if (!texture)
	{
		std::cerr << "Unable to create texture! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// 清屏
	SDL_RenderClear(renderer);

	// 渲染图片
	SDL_RenderTexture(renderer,
					  texture,
					  nullptr,
					  nullptr);

	// 更新屏幕
	SDL_RenderPresent(renderer);

	// 延时等待
	SDL_Delay(5000);

	// 清理资源
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


