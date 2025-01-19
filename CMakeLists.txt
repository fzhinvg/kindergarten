cmake_minimum_required(VERSION 3.29)

set(PROJECT_NAME kindergarten_local)
project(${PROJECT_NAME})

# 将即 libgcc 和 libstdc++ 以静态链接的方式嵌入到可执行文件中,主要是为了解决发给别人,结果人家告诉我缺少libc++.dll的问题
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# 此处显式指定编译器,不涉及隐私也能表示我用的是哪个发行版和版本
#set(CMAKE_C_COMPILER "E:/llvm_mingw/llvm-mingw-20241119-ucrt-x86_64/bin/clang.exe")
#set(CMAKE_CXX_COMPILER "E:/llvm_mingw/llvm-mingw-20241119-ucrt-x86_64/bin/clang++.exe")

# 设置安装前缀为根目录 ${CMAKE_SOURCE_DIR} 是 CMake 的一个内置变量,将安装文件放入的文件夹根目录,这一句确切指定文件夹路径,这里设置为根目录
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}") # 这一句还不能跟对应的那一句写到一起,有先后顺序一说,尽量保持这个文件的顺序吧,至少从现在(2025年1月20日)看,还是蛮层次清晰的

set(CMAKE_CXX_STANDARD 23)

#find_package(OpenMP REQUIRED)

include_directories(TemplateChallenge)

add_executable(${PROJECT_NAME}
        main.cpp
        TemplateChallenge/kit.h) # 我其实不是很理解为什么CLion要给我自动添加这一句,AI告诉我使用特定IDE可以提升开发体验 ( ´_ゝ` )

#target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX) # openmp也需要手动添加
# -g 调试符号信息 -O0 禁用优化
#target_compile_options(${PROJECT_NAME} PRIVATE -g -O0 -fexec-charset=GBK -finput-charset=UTF-8) # mingw 我的mingw没选多线程
#target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp  -g -O0) # with OpenMP
#target_compile_options(${PROJECT_NAME} PRIVATE -g -O0) # llvm-mingw

# 设置子系统为 Windows 以关闭控制台显示,仅在一些windows相关代码中启用
#if (WIN32) # 这个判断好像显得有点多余
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            LINK_FLAGS "-Wl,/subsystem:windows" # Wl 是一个链接器选项前缀,将后面的选项传递给链接器
#    )
#endif()

# 这里的文件名称可以修改,但是听说bin更加标准 # 这里是给手动编译使用的,将安装文件放入的文件夹名,两个语句共同作用才导致,这一句只决定文件夹名
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
