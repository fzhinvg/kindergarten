# 设置项目目录和安装目录
$projectDir = Get-Location
$buildDir = Join-Path $projectDir "build"

# 生成构建文件
# cmake -S . -B build
cmake -S $projectDir -B $buildDir

# 构建项目
cmake --build $buildDir --config Release

# 安装项目
cmake --install $buildDir

Write-Host "<<<.ps1 execution done!>>>"

# ===---------------------------------------------===

# 从手动编译看使用cmake编译的过程:

# 1.生成构建文件(Configuration)
# 在这一阶段,CMake 读取 CMakeLists.txt 文件,
# 并生成构建系统所需的文件(如 Makefile,Ninja 文件等)
# 此时没有任何源码被编译,生成的文件只是为了指导后续的编译过程.
# cmake -S . -B build
# 输入:源代码目录和 CMakeLists.txt
# 输出:构建系统文件(存放在 build 文件夹中)

# 2.构建项目(Build)
# 在这一阶段,CMake 使用之前生成的构建系统文件,
# 实际编译源代码,生成目标文件和可执行文件.
# cmake --build build --config Release
# 输入: 构建系统文件(存放在 build 文件夹中)
# 输出: 编译后的目标文件和可执行文件(也存放在 build 文件夹中)

# 3.安装项目(Install)
# 在这一阶段,CMake 根据 install 规则
# 将生成的可执行文件和其他必要文件复制到统一的安装目录
# 这个过程使项目的部署和分发更加规范和便捷
# cmake --install build
# 输入:编译后的目标文件和可执行文件(存放在 build 文件夹中)
# 输出:安装后的文件(根据 CMakeLists.txt 中的 install 规则,通常存放在指定的安装目录
# 还包括生成的 cmake_install.cmake 文件中的设置)

# CLion 主要自动化了前两个步骤
# 对于第三步,可以使用自定义的构建脚本(如 PowerShell 脚本)
# 至于install的位置,cmake会打印提示,
# 但是实际上就是CMakeLists中那两条语句指定的位置
# 针对本仓库就是根目录的bin文件夹
# clion好像并不会完全依赖于我设置的所有编译选项,
# 不过在他的build文件夹里它完成了12两步骤,
# 所以我可以直接使用 cmake --install . 来手动install
# 或者是 cmake --install cmake-build-debug

# ===---------------------------------------------===
