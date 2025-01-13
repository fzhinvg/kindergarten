//
// Created by fzhinvg on 2025/1/14.
//
#include <numeric>
#include <vector>
#include <iostream>
#include <execution>

int main()
{
#pragma region easy
	std::vector<int> vec = {1, 2, 3, 4, 5};

	// 使用std::reduce进行求和，初始值为0
	int sum = std::reduce(vec.begin(), vec.end(), 0);
	std::cout << "Sum: " << sum << std::endl;
	// std::reduce 的名字可能会让人联想到 "减少" 或 "简化"
	// 但它的实际行为是将一个范围内的元素通过指定的二元操作函数进行归约
	// 这种归约操作可以是累加,乘积,最大值,最小值等.
	// 之所以表现为累加行为,是因为默认情况下它使用 operator+ 作为二元操作函数,这就导致了累加的效果

	std::vector<int> vec2 = {1, 2, 3, 4, 5};

	// 使用自定义二元操作进行乘积计算，初始值为1
	int product = std::reduce(vec2.begin(), vec2.end(), 1, std::multiplies<int>());
	std::cout << "Product: " << product << std::endl;

	// 在 std::reduce 和 std::accumulate 中,初始值作为第0个元素用于第一次归约操作
	// 如果使用自定义的二元操作函数,通常必须指定一个初始值

	// std::accumulate 与 std::reduce 类似,它也会将范围内的所有元素通过一个二元操作函数进行归约
	// 不过 std::accumulate 总是按顺序从左到右进行累积操作,而 std::reduce 可以选择并行执行,顺序不固定
	// std::reduce 提供并行执行的支持,std::accumulate 只支持顺序执行

	// 使用std::accumulate进行求和
	int sum_acc = std::accumulate(vec.begin(), vec.end(), 0);
	std::cout << "Sum_acc: " << sum_acc << std::endl;

	// 使用自定义二元操作进行乘积计算
	int product_acc = std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<int>());
	std::cout << "Product: " << product_acc << std::endl;
#pragma endregion

#pragma region normal

	// binary_op 二元操作,所以可以是个两个参数的函数指针

	// 命名的lambda:
	auto binary_op = [](int a, int b)
	{ return (a << 1) | (b >> 1); };

	std::vector<int> vec_normal = {1, 2, 3, 4, 5};

	// 使用命名的 lambda 表达式进行二元操作
	int result = std::accumulate(vec_normal.begin(), vec_normal.end(), 0, binary_op);
	std::cout << "result: " << result << std::endl;
//	初始值 init = 0。
//	binary_op(0, 1) -> (0 << 1) | (1 >> 1) -> 0 | 0 -> 0
//	binary_op(0, 2) -> (0 << 1) | (2 >> 1) -> 0 | 1 -> 1
//	binary_op(1, 3) -> (1 << 1) | (3 >> 1) -> 2 | 1 -> 3
//	binary_op(3, 4) -> (3 << 1) | (4 >> 1) -> 6 | 2 -> 6
//	binary_op(6, 5) -> (6 << 1) | (5 >> 1) -> 12 | 2 -> 14

#pragma endregion

#pragma region lambda_expand
	auto la = [&]
	{ vec; };// [=] 按值捕获外部作用域中的所有变量 [&] 按引用捕获外部作用域中的所有变量 --关于这种lambda捕获方式的补充

	// 按值捕获指针
	int x = 10;
	int* ptr = &x;

	auto lambda = [ptr]() {
		std::cout << "Pointer value: " << *ptr << std::endl;
	};
	lambda();
	x=20;
	lambda();

	// 按引用捕获指针
	int xx = 10;
	int* ptr_xx = &xx;

	auto lambda_xx = [&ptr_xx]() {
		std::cout << "Pointer value: " << *ptr_xx << std::endl;
	};
	lambda_xx();
	xx = 20;
	lambda_xx();

	// 行为的区别就在于是不是新建一个指针,按值就新建,按引用就不

#pragma endregion

	return 0;
}
// 现在我意识到一个可能的问题,我目前装的mingw是不带多线程的,当时我还不知道这意味着什么
// 但是这也不是重点,重点是我现在知道了clang和gcc的一个区别
// clang用的标准库实现叫:libc++,它是LLVM项目的标准库实现
// gcc提供的标准库实现是:libstdc++
// 所以现在,我的mingw可能无法使用并行执行策略
// 但是我的clang本身又对执行策略支持有限 ( ´_ゝ｀)那很坏了