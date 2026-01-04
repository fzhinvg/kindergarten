// todo : macos兼容, linux兼容, 渲染置顶, 多屏幕兼容

#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <atomic>
#include <climits>
#include <fstream>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "json.hpp"
using json = nlohmann::json;

#ifdef _WIN32
#include <windows.h>
#endif

class MouseTrail {
private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    // 鼠标轨迹点结构
    struct TrailPoint {
        float x, y;
        float alpha;    // 透明度
        float size;     // 点大小
        uint64_t timestamp; // 时间戳
    };

    std::deque<TrailPoint> trailPoints;
    size_t maxTrailPoints = 50; // 从配置文件读取

    // 颜色渐变（从配置文件读取）
    SDL_Color startColor = {255, 100, 100, 255};
    SDL_Color endColor = {100, 100, 255, 255};

    std::atomic<bool> running = true;
    uint64_t lastUpdateTime = 0;
    uint64_t updateInterval = 16; // 从配置文件读取

    // 窗口位置和大小（覆盖所有显示器）
    int windowX = 0;
    int windowY = 0;
    int windowWidth = 0;
    int windowHeight = 0;

    // 上一次鼠标位置
    float lastMouseX = -1;
    float lastMouseY = -1;

    // 鼠标移动检测阈值（从配置文件读取）
    float positionThreshold = 0.5f;

    // 绘制开关
    std::atomic<bool> drawingEnabled = true;

    // 新增：是否绘制连接线（从配置文件读取）
    bool drawConnectingLinesEnabled = true;

    // 新增：拖尾变化速度参数（从配置文件读取）
    float fadeSpeed = 1.5f;      // 透明度衰减速度
    float shrinkSpeed = 15.0f;   // 大小缩小速度
    float initialAlpha = 1.0f;   // 初始透明度
    float initialSize = 20.0f;   // 初始大小
    float lineAlphaMultiplier = 150.0f; // 连接线透明度系数

#ifdef _WIN32
    HHOOK keyboardHook = nullptr;

    // 组合键状态跟踪
    bool toggleComboTriggered = false;   // 用于Alt+D切换绘制
    bool exitComboTriggered = false;     // 用于Alt+T退出程序
#endif

    // 虚拟桌面信息
    struct MonitorInfo {
        int x, y, width, height;
    };
    std::vector<MonitorInfo> monitors;

    // 配置文件路径
    std::string configFile = "config.json";

public:
    bool init() {
        // 加载配置文件
        if (!loadConfig()) {
            std::cerr << "Failed to load config, using default values" << std::endl;
        }

        // 保存默认配置（如果文件不存在）
        saveDefaultConfig();

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }

        // Get all monitor information
        if (!getAllMonitorsInfo()) {
            std::cerr << "Failed to get monitor information: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        std::cout << "Found " << monitors.size() << " monitor(s)" << std::endl;
        for (size_t i = 0; i < monitors.size(); ++i) {
            std::cout << "Monitor " << i << ": position(" << monitors[i].x << ", " << monitors[i].y
                      << ") size(" << monitors[i].width << "x" << monitors[i].height << ")" << std::endl;
        }

        // Calculate virtual desktop covering all monitors
        calculateVirtualDesktop();

        // Create borderless transparent window
        SDL_PropertiesID props = SDL_CreateProperties();
        if (!props) {
            std::cerr << "Failed to create window properties: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "MouseTrail");
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, windowX);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, windowY);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, windowWidth);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, windowHeight);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_TRANSPARENT_BOOLEAN, true);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, true);
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_UTILITY_BOOLEAN, true);

        window = SDL_CreateWindowWithProperties(props);
        SDL_DestroyProperties(props);

        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        // Create renderer
        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }

        // Set blend mode for transparency
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Set window opacity
        if (SDL_SetWindowOpacity(window, 1.0f) < 0) {
            std::cerr << "Warning: Could not set window opacity: " << SDL_GetError() << std::endl;
        }

        // Set window click-through (Windows specific)
        setWindowClickThrough();

        // Install keyboard hook
        if (!installKeyboardHook()) {
            std::cerr << "Warning: Could not install keyboard hook" << std::endl;
        }

        std::cout << "Mouse trail program started" << std::endl;
        std::cout << "Window position: (" << windowX << ", " << windowY << ")" << std::endl;
        std::cout << "Window size: " << windowWidth << "x" << windowHeight << std::endl;
        std::cout << "Drawing connecting lines: " << (drawConnectingLinesEnabled ? "YES" : "NO") << std::endl;
        std::cout << "Fade speed: " << fadeSpeed << std::endl;
        std::cout << "Shrink speed: " << shrinkSpeed << std::endl;
        std::cout << "    Press Alt+D to toggle drawing" << std::endl;
        std::cout << "    Press Alt+T to exit" << std::endl;

        return true;
    }

    // 加载配置文件
    bool loadConfig() {
        std::ifstream file(configFile);
        if (!file.is_open()) {
            std::cerr << "Config file not found: " << configFile << std::endl;
            return false;
        }

        try {
            json config;
            file >> config;

            // 读取颜色配置
            if (config.contains("start_color")) {
                auto color = config["start_color"];
                startColor.r = color[0];
                startColor.g = color[1];
                startColor.b = color[2];
                startColor.a = color[3];
                std::cout << "Start color: R=" << (int)startColor.r
                          << " G=" << (int)startColor.g
                          << " B=" << (int)startColor.b
                          << " A=" << (int)startColor.a << std::endl;
            }

            if (config.contains("end_color")) {
                auto color = config["end_color"];
                endColor.r = color[0];
                endColor.g = color[1];
                endColor.b = color[2];
                endColor.a = color[3];
                std::cout << "End color: R=" << (int)endColor.r
                          << " G=" << (int)endColor.g
                          << " B=" << (int)endColor.b
                          << " A=" << (int)endColor.a << std::endl;
            }

            // 读取轨迹参数
            if (config.contains("draw_connecting_lines")) {
                drawConnectingLinesEnabled = config["draw_connecting_lines"];
                std::cout << "Draw connecting lines: " << drawConnectingLinesEnabled << std::endl;
            }

            if (config.contains("max_trail_points")) {
                maxTrailPoints = config["max_trail_points"];
                std::cout << "Max trail points: " << maxTrailPoints << std::endl;
            }

            if (config.contains("update_interval")) {
                updateInterval = config["update_interval"];
                std::cout << "Update interval: " << updateInterval << "ms" << std::endl;
            }

            if (config.contains("mouse_move_threshold")) {
                positionThreshold = config["mouse_move_threshold"];
                std::cout << "Mouse move threshold: " << positionThreshold << "px" << std::endl;
            }

            // 读取拖尾变化速度参数
            if (config.contains("fade_speed")) {
                fadeSpeed = config["fade_speed"];
                std::cout << "Fade speed: " << fadeSpeed << std::endl;
            }

            if (config.contains("shrink_speed")) {
                shrinkSpeed = config["shrink_speed"];
                std::cout << "Shrink speed: " << shrinkSpeed << std::endl;
            }

            if (config.contains("initial_alpha")) {
                initialAlpha = config["initial_alpha"];
                std::cout << "Initial alpha: " << initialAlpha << std::endl;
            }

            if (config.contains("initial_size")) {
                initialSize = config["initial_size"];
                std::cout << "Initial size: " << initialSize << std::endl;
            }

            if (config.contains("line_alpha_multiplier")) {
                lineAlphaMultiplier = config["line_alpha_multiplier"];
                std::cout << "Line alpha multiplier: " << lineAlphaMultiplier << std::endl;
            }

            return true;

        } catch (const json::exception& e) {
            std::cerr << "Error parsing config file: " << e.what() << std::endl;
            return false;
        }
    }

    // 保存默认配置文件
    void saveDefaultConfig() {
        std::ifstream testFile(configFile);
        if (testFile.is_open()) {
            testFile.close();
            return; // 文件已存在
        }

        json defaultConfig = {
            {"start_color", {255, 100, 100, 255}},
            {"end_color", {100, 100, 255, 255}},
            {"draw_connecting_lines", true},
            {"max_trail_points", 50},
            {"update_interval", 16},
            {"mouse_move_threshold", 0.5},
            {"fade_speed", 1.5},
            {"shrink_speed", 15.0},
            {"initial_alpha", 1.0},
            {"initial_size", 20.0},
            {"line_alpha_multiplier", 150.0}
        };

        std::ofstream outFile(configFile);
        if (outFile.is_open()) {
            outFile << defaultConfig.dump(4); // 使用缩进格式化输出
            std::cout << "Default config file created: " << configFile << std::endl;
        } else {
            std::cerr << "Failed to create config file: " << configFile << std::endl;
        }
    }

    // Get all monitor information
    bool getAllMonitorsInfo() {
        SDL_DisplayID* displays = SDL_GetDisplays(nullptr);
        if (!displays) {
            std::cerr << "Failed to get display list: " << SDL_GetError() << std::endl;
            return false;
        }

        int numDisplays = 0;
        for (SDL_DisplayID* p = displays; *p; ++p) {
            numDisplays++;
        }

        for (int i = 0; i < numDisplays; ++i) {
            SDL_DisplayID display = displays[i];
            const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
            if (!mode) {
                std::cerr << "Failed to get mode for display " << i << ": " << SDL_GetError() << std::endl;
                continue;
            }

            // Get display bounds
            SDL_Rect bounds;
            if (SDL_GetDisplayBounds(display, &bounds) != 0) {
                std::cerr << "Failed to get bounds for display " << i << ": " << SDL_GetError() << std::endl;
                // Use display mode as fallback
                bounds.x = 0;
                bounds.y = 0;
                bounds.w = mode->w;
                bounds.h = mode->h;
            }

            MonitorInfo info;
            info.x = bounds.x;
            info.y = bounds.y;
            info.width = bounds.w;
            info.height = bounds.h;

            monitors.push_back(info);
        }

        SDL_free(displays);
        return !monitors.empty();
    }

    // Calculate virtual desktop rectangle (covering all monitors)
    void calculateVirtualDesktop() {
        if (monitors.empty()) {
            // Fallback to primary display if no monitor info
            SDL_DisplayID primary = SDL_GetPrimaryDisplay();
            const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(primary);
            if (mode) {
                windowX = 0;
                windowY = 0;
                windowWidth = mode->w;
                windowHeight = mode->h;
            }
            return;
        }

        // Calculate union rectangle of all monitors
        int minX = INT_MAX, minY = INT_MAX;
        int maxX = INT_MIN, maxY = INT_MIN;

        for (const auto& monitor : monitors) {
            minX = std::min(minX, monitor.x);
            minY = std::min(minY, monitor.y);
            maxX = std::max(maxX, monitor.x + monitor.width);
            maxY = std::max(maxY, monitor.y + monitor.height);
        }

        windowX = minX;
        windowY = minY;
        windowWidth = maxX - minX;
        windowHeight = maxY - minY;
    }

    // Set window click-through (Windows specific implementation)
    void setWindowClickThrough() {
#ifdef _WIN32
        // Find window by title
        HWND hwnd = FindWindowA(nullptr, "MouseTrail");
        if (!hwnd) {
            // Enumerate all top-level windows to find matching title
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                char title[256];
                GetWindowTextA(hwnd, title, sizeof(title));
                if (strstr(title, "MouseTrail")) {
                    *reinterpret_cast<HWND*>(lParam) = hwnd;
                    return FALSE; // Stop enumeration
                }
                return TRUE; // Continue enumeration
            }, reinterpret_cast<LPARAM>(&hwnd));
        }

        if (hwnd) {
            // Get current extended style
            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

            // Add transparent and layered window styles
            exStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

            // Set layered window attributes
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

            // Ensure window won't get focus
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            std::cout << "Window click-through set successfully" << std::endl;
        } else {
            std::cerr << "Could not find window handle, click-through may not work" << std::endl;

            // Fallback: use SDL's mouse ignore
            SDL_SetWindowMouseRect(window, nullptr);
            SDL_SetWindowMouseGrab(window, false);
        }
#else
        // Non-Windows fallback
        SDL_SetWindowMouseRect(window, nullptr);
        SDL_SetWindowMouseGrab(window, false);
#endif
    }

    // Install keyboard hook
    bool installKeyboardHook() {
#ifdef _WIN32
        // Install low-level keyboard hook
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
        if (!keyboardHook) {
            DWORD error = GetLastError();
            std::cerr << "SetWindowsHookEx failed with error: " << error << std::endl;
            std::cerr << "Try running the program as administrator" << std::endl;
            return false;
        }
        return true;
#else
        return false;
#endif
    }

    // Uninstall keyboard hook
    void uninstallKeyboardHook() {
#ifdef _WIN32
        if (keyboardHook) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = nullptr;
        }
#endif
    }

    // 键盘钩子回调函数（静态）
#ifdef _WIN32
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            // 所有键盘事件现在都通过checkHotkeys()处理
            // 这里保持钩子结构，但不做具体处理
        }

        // 将消息传递给下一个钩子
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
#endif

    // 检查热键（Alt+D 切换绘制，Alt+T 退出程序）
    void checkHotkeys() {
#ifdef _WIN32
        // 检查左Alt键
        bool currentLeftAltPressed = (GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;

        // 检查D键（切换绘制）
        bool currentDKeyPressed = (GetAsyncKeyState('D') & 0x8000) != 0;

        // 检查T键（退出程序）
        bool currentTKeyPressed = (GetAsyncKeyState('T') & 0x8000) != 0;

        // 如果左Alt键和D键同时被按下，并且之前没有触发
        if (currentLeftAltPressed && currentDKeyPressed && !toggleComboTriggered) {
            // 切换绘制开关
            drawingEnabled = !drawingEnabled;
            toggleComboTriggered = true;

            std::cout << "Drawing: " << (drawingEnabled ? "ENABLED" : "DISABLED") << std::endl;

            // 如果是关闭状态，清空现有轨迹点
            if (!drawingEnabled) {
                trailPoints.clear();
            }
        }

        // 如果左Alt键和T键同时被按下，并且之前没有触发
        if (currentLeftAltPressed && currentTKeyPressed && !exitComboTriggered) {
            // 设置退出标志
            running = false;
            exitComboTriggered = true;
            std::cout << "Alt+T pressed, exiting..." << std::endl;
        }

        // 如果组合键被释放，重置触发标志
        if (!currentLeftAltPressed || !currentDKeyPressed) {
            toggleComboTriggered = false;
        }
        if (!currentLeftAltPressed || !currentTKeyPressed) {
            exitComboTriggered = false;
        }
#endif
    }

    void run() {
        SDL_Event event;

        // 使用高分辨率计时器
        uint64_t lastFrameTime = SDL_GetTicks();
        uint64_t lastMouseUpdate = 0;
        uint64_t lastHotkeyCheck = 0;

#ifdef _WIN32
        // Windows消息循环
        MSG msg;
#endif

        while (running) {
            uint64_t currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastFrameTime) / 1000.0f;
            lastFrameTime = currentTime;

#ifdef _WIN32
            // 处理Windows消息（用于键盘钩子）
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    // 仍然处理WM_QUIT消息，但主要退出逻辑在checkHotkeys中
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
#endif

            // 处理SDL事件
            while (SDL_PollEvent(&event)) {
                handleEvent(event);
            }

            // 定期检查热键（避免检查太频繁）
            if (currentTime - lastHotkeyCheck > 50) { // 每50ms检查一次
                checkHotkeys();
                lastHotkeyCheck = currentTime;
            }

            // 定期更新鼠标位置（即使没有鼠标移动事件）
            // 只在绘制启用时更新
            if (drawingEnabled && currentTime - lastMouseUpdate > updateInterval) {
                updateMousePosition();
                lastMouseUpdate = currentTime;
            }

            // 更新鼠标轨迹
            updateTrail(deltaTime);

            // 渲染
            render();

            // 控制帧率
            SDL_Delay(1);
        }
    }

    void updateMousePosition() {
        // Get global mouse position (relative to virtual desktop)
        float mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        // Adjust mouse position to window coordinate system
        float adjustedX = mouseX - windowX;
        float adjustedY = mouseY - windowY;

        // Check if mouse has moved and drawing is enabled
        if (drawingEnabled && hasMouseMoved(adjustedX, adjustedY)) {
            addTrailPoint(adjustedX, adjustedY);
            // Update last mouse position
            lastMouseX = adjustedX;
            lastMouseY = adjustedY;
        } else if (drawingEnabled) {
            // Update position even if mouse hasn't moved (for next comparison)
            lastMouseX = adjustedX;
            lastMouseY = adjustedY;
        }
    }

    // Check if mouse has moved
    bool hasMouseMoved(float currentX, float currentY) {
        // If first call, return true
        if (lastMouseX < 0 || lastMouseY < 0) {
            return true;
        }

        // Calculate movement distance
        float dx = currentX - lastMouseX;
        float dy = currentY - lastMouseY;
        float distance = std::sqrt(dx * dx + dy * dy);

        // If movement distance exceeds threshold, mouse has moved
        return distance > positionThreshold;
    }

    void handleEvent(const SDL_Event &event) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                std::cout << "Received quit event" << std::endl;
                break;

            case SDL_EVENT_MOUSE_MOTION:
                // Only check mouse movement if drawing is enabled
                if (drawingEnabled) {
                    // Adjust mouse position to window coordinate system
                    float adjustedX = static_cast<float>(event.motion.x) - windowX;
                    float adjustedY = static_cast<float>(event.motion.y) - windowY;

                    if (hasMouseMoved(adjustedX, adjustedY)) {
                        // Add new trail point
                        addTrailPoint(adjustedX, adjustedY);
                        // Update last mouse position
                        lastMouseX = adjustedX;
                        lastMouseY = adjustedY;
                    } else {
                        // Update position even if mouse hasn't moved
                        lastMouseX = adjustedX;
                        lastMouseY = adjustedY;
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_WHEEL:
                // Ignore mouse click and wheel events, let them pass through
                break;
        }
    }

    void addTrailPoint(float x, float y) {
        uint64_t currentTime = SDL_GetTicks();

        // Limit addition frequency
        if (currentTime - lastUpdateTime < updateInterval) {
            return;
        }

        TrailPoint newPoint = {
            x, y,
            initialAlpha,  // 使用配置的初始透明度
            initialSize,   // 使用配置的初始大小
            currentTime
        };

        trailPoints.push_back(newPoint);

        // Limit number of trail points
        if (trailPoints.size() > maxTrailPoints) {
            trailPoints.pop_front();
        }

        lastUpdateTime = currentTime;
    }

    void updateTrail(float deltaTime) {
        uint64_t currentTime = SDL_GetTicks();

        // Update state of all trail points
        for (auto &point: trailPoints) {
            float age = (currentTime - point.timestamp) / 1000.0f; // Convert to seconds

            // Reduce transparency and size over time (使用配置的速度参数)
            point.alpha = std::max(0.0f, initialAlpha - age * fadeSpeed);
            point.size = std::max(5.0f, initialSize - age * shrinkSpeed);
        }

        // Remove completely transparent points
        trailPoints.erase(
            std::remove_if(trailPoints.begin(), trailPoints.end(),
                           [](const TrailPoint &p) { return p.alpha <= 0.01f; }),
            trailPoints.end()
        );
    }

    void render() {
        // Clear screen (completely transparent)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        if (trailPoints.empty()) {
            SDL_RenderPresent(renderer);
            return;
        }

        // 只有在启用时才绘制连接线
        if (drawConnectingLinesEnabled) {
            drawConnectingLines();
        }

        // Draw trail points
        for (const auto &point: trailPoints) {
            drawTrailPoint(point);
        }

        // Present render result
        SDL_RenderPresent(renderer);
    }

    void drawTrailPoint(const TrailPoint &point) {
        // Calculate color interpolation
        float t = 1.0f - point.alpha;

        SDL_Color color;
        color.r = static_cast<Uint8>(startColor.r * (1 - t) + endColor.r * t);
        color.g = static_cast<Uint8>(startColor.g * (1 - t) + endColor.g * t);
        color.b = static_cast<Uint8>(startColor.b * (1 - t) + endColor.b * t);
        color.a = static_cast<Uint8>(point.alpha * 255);

        // Draw circular point
        drawCircle(point.x, point.y, point.size / 2, color);
    }

    void drawCircle(float centerX, float centerY, float radius, SDL_Color color) {
        int r = static_cast<int>(radius);
        int cx = static_cast<int>(centerX);
        int cy = static_cast<int>(centerY);

        // Optimize: only draw points inside the circle
        int minX = std::max(0, cx - r);
        int maxX = std::min(windowWidth, cx + r);
        int minY = std::max(0, cy - r);
        int maxY = std::min(windowHeight, cy + r);

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                float dx = x - centerX;
                float dy = y - centerY;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= radius) {
                    // Calculate alpha value based on distance (create gradient effect)
                    float alphaFactor = 1.0f - (distance / radius) * 0.5f;
                    Uint8 alpha = static_cast<Uint8>(color.a * alphaFactor);

                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
                    SDL_RenderPoint(renderer, static_cast<float>(x), static_cast<float>(y));
                }
            }
        }
    }

    void drawConnectingLines() {
        if (trailPoints.size() < 2) return;

        // Draw connecting lines
        for (size_t i = 0; i < trailPoints.size() - 1; i++) {
            const auto &p1 = trailPoints[i];
            const auto &p2 = trailPoints[i + 1];

            // Line transparency is average of two points' transparency
            float alpha = (p1.alpha + p2.alpha) / 2.0f;

            // Calculate interpolated color
            float t = 1.0f - alpha;
            SDL_Color color;
            color.r = static_cast<Uint8>(startColor.r * (1 - t) + endColor.r * t);
            color.g = static_cast<Uint8>(startColor.g * (1 - t) + endColor.g * t);
            color.b = static_cast<Uint8>(startColor.b * (1 - t) + endColor.b * t);
            color.a = static_cast<Uint8>(alpha * lineAlphaMultiplier); // 使用配置的透明度系数

            // Draw line
            drawLine(p1.x, p1.y, p2.x, p2.y, color);
        }
    }

    void drawLine(float x1, float y1, float x2, float y2, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderLine(renderer, x1, y1, x2, y2);
    }

    void cleanup() {
        // Uninstall keyboard hook
        uninstallKeyboardHook();

        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    ~MouseTrail() {
        cleanup();
    }
};

int main(int argc, char *argv[]) {
    MouseTrail app;

    if (app.init()) {
        app.run();
    }

    return 0;
}