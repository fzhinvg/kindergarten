# kindergarten项目文件结构&CMakeLists模板简介
## 项目结构

```
项目根目录/
├── CMakeLists.txt
├── include/          # 头文件 (.h)
├── source/           # 源文件 (.cpp) 和类文件
├── resources/        # 资源文件
├── lib/
│   ├── static/       # 静态库 (.a)
│   └── shared/       # 动态库 (.dll.a / .so / .dylib) 和运行时 .dll
├── bin/              # 编译输出目录（自动生成）
└── kindergarten.rc   # Windows 资源文件
```

---

## 添加源文件

直接将 `.cpp` 文件放入 `source/` 文件夹，CMake 通过 `GLOB_RECURSE` 自动收集，无需修改 CMakeLists.txt。

```
source/
├── main.cpp
├── Player.cpp        ← 直接放进来即可
└── Enemy.cpp         ← 直接放进来即可
```

> **注意**：新增文件后需要在 CLion 中 **Reload CMake Project**，才能被识别。

---

## 添加类文件

头文件和实现文件均可放在 `source/` 下统一管理：

```
source/
├── Player.h
├── Player.cpp
```

也可以将头文件放在 `include/` 下：

```
include/
└── Player.h
source/
└── Player.cpp
```

两种方式均已在 `include_directories` 中配置，`#include` 时直接写文件名即可：

```cpp
#include "Player.h"
```

---

---

## 链接静态库(未测试)

将 `.a` 文件放入 `lib/static/`，CMake 自动链接，无需修改 CMakeLists.txt。

```
lib/
└── static/
    └── libFoo.a      ← 放进来即可
```

---

## 链接动态库

### Windows

将以下两个文件放入 `lib/shared/`：

| 文件 | 用途 |
|------|------|
| `libFoo.dll.a` | 链接时使用的导入库 |
| `Foo.dll` | 运行时依赖，构建后自动复制到 `bin/` |

```
lib/
└── shared/
    ├── libFoo.dll.a  ← 链接用
    └── Foo.dll       ← 运行时用，自动复制到 bin/
```

### Linux

将 `.so` 文件放入 `lib/shared/`：

```
lib/
└── shared/
    └── libFoo.so
```

### macOS

将 `.dylib` 文件放入 `lib/shared/`：

```
lib/
└── shared/
    └── libFoo.dylib
```

---

## 更换项目名称

修改 CMakeLists.txt 第一行即可：

```cmake
set(PROJECT_NAME 你的项目名)
```

---

## 重新加载 CMake

以下情况需要在 CLion 中手动 **Reload CMake Project**：

- 向 `source/` 新增或删除 `.cpp` 文件
- 向 `lib/` 新增或删除库文件
