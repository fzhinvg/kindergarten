//
// Created by fzhinvg on 2024/12/30.
//
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>


// 动态规划求解斐波那契数列
int fibonacci(int n)
{
	if (n <= 1)
	{
		return n;
	}
	std::vector<int> dp(n + 1);
	dp[0] = 0;
	dp[1] = 1;
	for (int i = 2; i <= n; ++i)
	{
		dp[i] = dp[i - 1] + dp[i - 2];
	}
	return dp[n];
}
// 动态规划的核心思想,设计和解析思路

// 1.重叠子问题:通过将问题分解成多个子问题,并发现这些子问题之间的重叠性
// 在fibonacci()中,计算 F(n) 需要F(n-1)和F(n-2),而F(n-1)需要 F(n-2)和F(n-3),这些子问题重叠

// 2.最优子结构:一个问题的最优解可以由其子问题的最优解来构造
// F(n)=F(n-1)+F(n-2),即每个状态F(n)可以由前两个状态F(n-1)和F(n-2)的最优解来构造

// 3.状态转移方程:定义了从一个状态到另一个状态的转换关系
// dp[i]=dp[i-1]+dp[i-2]是fibonacci()的状态转移方程,从状态i-1和i-2转移到状态i

// 动态规划的核心机制——记忆化Memoization和表格法Tabulation
// fibonacci()中使用一个vector容器来存储中间结果


// 经典动态规划算法求解最长递增子序列
int lengthOfLIS(const std::vector<int> &nums)
{
	if (nums.empty())
	{
		return 0;
	}
	std::vector<int> dp(nums.size(), 1);

	for (size_t i = 1; i < nums.size(); ++i)
	{
		for (size_t j = 0; j < i; ++j)
		{
			if (nums[i] > nums[j])
			{
				dp[i] = std::max(dp[i], dp[j] + 1);
			}
		}
	}
	for (auto item: dp)
	{
		std::cout << item << " ";
	}
	std::cout << std::endl;
	return *std::max_element(dp.begin(), dp.end());
}


//int lengthOfLIS_alter(const std::vector<int> &sequence)
//{
//	if (sequence.empty())
//	{
//		return 0;
//	}
//	std::vector<int> lisLengths(sequence.size(), 1);
//	// 它是相对于每一个元素作为子序列结尾构造它能拥有的最长子序列长度序列,这个序列就是lisLengths[1, 1, 1, 2, 2, 3, 4, 4]
//
//	for (size_t currentIndex = 1; currentIndex < sequence.size(); ++currentIndex)
//	{
//		for (size_t previousIndex = 0; previousIndex < currentIndex; ++previousIndex)
//		{
//			if (sequence[currentIndex] > sequence[previousIndex])
//			{
//				lisLengths[currentIndex] = std::max(lisLengths[currentIndex], lisLengths[previousIndex] + 1);
//			}
//			// 这个算法对于每个数构造的子序列长度就像阶梯一样递增
//			// 后一个数字如果能当前一个数字的结尾,说明他能构造的子序列长度就是前一个数字能构造的子序列长度加一
//			// 所以核心就是如果[a,b,c]中,原本是[1,init,init]如果a<b,说明构成递增
//			// 转化为[1,2,init],接着如果b>c则说明不构成递增,就转化为[1,2,2],以此类推
//		}
//	}
//	return *std::max_element(lisLengths.begin(), lisLengths.end());
//}
// 我感觉它的精巧之处是在于,能找到这么一个方法来表示是否能成立递增子序列,我想大多数实战的时候,
// 我们是很难想出来这个"后一个数字如果能当前一个数字的结尾,说明他能构造的子序列长度就是前一个数字能构造的子序列长度加一"这个算法构造逻辑的.

// 我尝试将它简单地表现出来
// 这是一个失败的作品,我的行为像是在计算整个序列拥有多少个上升趋势,我的设想是错误的
// 我的狂妄让我以为自己找到了什么更简单的的解决方法,实际上是我对这个问题的了解过于片面和
// 对于问题的分支决策不经思考的舍弃,这是很不好的行为
int lengthOfLIS_new(const std::vector<int> &sequence) //int countIncrements()
{
	// inheritance_length 继承已经遍历序列能构造出的最长子序列长度
	std::vector<int> inheritance_length(sequence.size(), 1);
	for (size_t index = 0; index < sequence.size(); ++index)
	{
		static int prev = -1;
		if (index == 0)
		{
			prev = sequence[index];
		} else
		{
			if (prev < sequence[index])
			{
				inheritance_length[index] = inheritance_length[index - 1] + 1;
			} else
			{
				inheritance_length[index] = inheritance_length[index - 1];
			}
			prev = sequence[index];
		}
	}
	for (auto item: inheritance_length)
	{
		std::cout << item << " ";
	}
	std::cout << std::endl;
	return *std::max_element(inheritance_length.begin(), inheritance_length.end());
}
// 但是这个function可以用来计算整个序列有多少个上升,不是吗
// 就像是开心消消乐,不同的连击等级,达到的不同消除评价,触发和满足暴击条件,就上调一个评价等级


std::vector<int> generateRandomSequence(int length, int maxValue)
{
	std::vector<int> sequence(length);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, maxValue);

	for (int i = 0; i < length; ++i)
	{
		sequence[i] = dis(gen);
	}
	return sequence;
}


int main()
{
//	int n = 10;
//	std::cout << "Fibonacci(" << n << ") = " << fibonacci(n) << std::endl;

//	std::vector<int> nums = {10, 9, 2, 5, 3, 7, 101, 18};
//	int length = lengthOfLIS(nums); // 2 3 7 101 or 2 3 7 18
//	std::cout << "Length of Longest Increasing Subsequence: " << length << std::endl;
//	std::cout << lengthOfLIS_new(nums) << std::endl;

	for (int time = 0; time < 10; ++time)
	{
		int length = 10;
		int maxValue = 100;
		std::vector<int> sequence = generateRandomSequence(length, maxValue);

		std::cout << "Random Sequence: ";
		for (const int &num: sequence)
		{
			std::cout << num << " ";
		}
		std::cout << std::endl;

		int result_new = lengthOfLIS_new(sequence);
		int result_classic = lengthOfLIS(sequence);
		std::cout << "Result from lengthOfLIS_new: " << result_new << std::endl;
		std::cout << "Result from lengthOfLIS (classic DP): " << result_classic << std::endl;

		if (result_new == result_classic)
		{
			std::cout << "Both algorithms return the same result." << std::endl;
		} else
		{
			std::cout << "My version is shit." << std::endl;
		}
	}

	return 0;
}
