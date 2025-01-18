cmake_minimum_required(VERSION 3.29)

set(PROJECT_NAME kindergarten_local)
project(${PROJECT_NAME})

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
target_compile_options(${PROJECT_NAME} PRIVATE -g -O0) # llvm-mingw
