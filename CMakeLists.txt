cmake_minimum_required(VERSION 3.29)

set(PROJECT_NAME kindergarten_local)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}
        main.cpp)

# -g 调试符号信息 -O0 禁用优化
#target_compile_options(${PROJECT_NAME} PRIVATE -g -O0 -fexec-charset=GBK -finput-charset=UTF-8) # mingw 我的mingw没选多线程
target_compile_options(${PROJECT_NAME} PRIVATE -g -O0) # llvm-mingw
