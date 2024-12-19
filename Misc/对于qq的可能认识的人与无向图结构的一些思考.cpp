//
// Created by fzhinvg on 2024/12/19.
//
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>
#include <set>

class Graph
{
private:
	std::unordered_map<int, std::unordered_set<int>> adjList; // [node],{The set of nodes linked to this node}
public:
	void addEdge(int u, int v)
	{
		adjList[u].insert(v);
		adjList[v].insert(u); // 一条边,给两个节点添加链接信息
	}

	// ai告诉我可以这种graph可以用于社交网络好友推荐,我的想法是
	// 按照隐私惯例,每个用户节点只应该能看到路径深度为1的节点和关系,
	// 节点就是其他user,关系就是边,可以给边分为不同属性,这里方便全部用int表示
	// 那么推荐好友就是直接推荐传入节点的路径深度为2的其他节点列表
	// 我想到一个问题和解决方案:如何在遍历无向图时避免回溯到已经访问过的节点,像蚂蚁一样留下自己的信息素
	[[nodiscard]] std::vector<int> recommendFriends(int user) const // BFS
	{
		std::unordered_set<int> recommended; // 推荐的朋友
		std::queue<int> nodeQueue;           // 进行BFS的队列
		std::unordered_set<int> visited;     // 用于记录已访问的节点

		// 初始化广度优先搜索
		nodeQueue.push(user);
		visited.insert(user);
		int current_depth = 0;

		while (!nodeQueue.empty() && current_depth < 2)
		{
			size_t levelSize = nodeQueue.size();
			current_depth++;
			for (size_t i = 0; i < levelSize; ++i)
			{
				int node = nodeQueue.front();
				nodeQueue.pop();
				for (int neighbor: adjList.at(node))
				{
					if (visited.find(neighbor) == visited.end())
					{
						visited.insert(neighbor);
						nodeQueue.push(neighbor);
						if (current_depth == 2)
						{
							recommended.insert(neighbor); // 添加深度为2的节点到推荐列表
						}
					}
				}
			}
		}

		return std::vector<int>(recommended.begin(), recommended.end());
	} // 有一种小脑缺陷的美,明明深度为2直接用边当成key传入adjList,找到对应的边,全部推入一个unique容器就行

	[[nodiscard]] std::vector<int> recommendFriendsAlter(int user) const
	{
		std::unordered_set<int> recommended;

		// 获取直接朋友 keys
		const auto &friends = adjList.at(user); // adjList[user]
		// [key]{set}
		// 获取朋友的朋友
		for (int friendID: friends)
		{
			const auto &friendsOfFriend = adjList.at(friendID);
			for (int potentialFriend: friendsOfFriend)
			{
				if (potentialFriend != user && friends.find(potentialFriend) == friends.end())
				{
					recommended.insert(potentialFriend);
				}
			}
		}

		// 转换为 vector 返回
		return std::vector<int>(recommended.begin(), recommended.end());
	}

	// 我好像忽略了一个点，就是潜在的朋友应该是有共同好友的，现在我只是查找了朋友的朋友，
	// 而忽略了他们应该有共同好友这一点，但是我感觉可以修改一下使用的容器
	// 用一个unordered_map<int,int>,来解决，第一个int是潜在好友，第二个是相同好友的人数。
	[[nodiscard]] std::unordered_map<int, int> recommendFriendsAndRefCount(int user) const
	{
		std::unordered_map<int, int> recommended; // 存储潜在朋友及其共同好友数量

		const auto &friends = adjList.at(user);

		for (int friendID: friends)
		{
			const auto &friendsOfFriend = adjList.at(friendID);
			for (int potentialFriend: friendsOfFriend)
			{
				if (potentialFriend != user && friends.find(potentialFriend) == friends.end())
				{
					recommended[potentialFriend]++;
				}
			}
		}

		return recommended;
	}

	void DFS(int node, std::unordered_set<int> &visited) const// visited 需要被每一层函数共享
	{
		if (visited.find(node) != visited.end())
		{
			return; // 已访问
		}

		visited.insert(node);
		std::cout << "Visited node: " << node << std::endl;

		for (int neighbor: adjList.at(node))
		{
			DFS(neighbor, visited);
		}
	}

	int countUniqueEdges() const
	{
		std::set<std::pair<int, int>> uniqueEdges;

		for (const auto &node: adjList)
		{
			int u = node.first;
			for (int v: node.second)
			{
				if (u < v)
				{
					uniqueEdges.insert({u, v});
				} else
				{
					uniqueEdges.insert({v, u});
				}
			}
		}
		return uniqueEdges.size();
	}

};

int main()
{
	Graph g;
	g.addEdge(1, 2);
	g.addEdge(1, 3);
	g.addEdge(2, 4);
	g.addEdge(3, 4);
	g.addEdge(4, 5);

	// Graph:
	//    1
	//   / \
	//  2   3
	//   \ /
	//    4
	//    |
	//    5

	int target = 5;
	std::vector<int> recommendations = g.recommendFriends(target);
	std::cout << "Friend recommendations for user " << target << ": " << std::endl;
	for (int rec: recommendations)
	{
		std::cout << rec << std::endl;
	}

	auto recommendedList = g.recommendFriendsAlter(target);
	std::cout << "recommended list:" << std::endl;
	for (auto &item: recommendedList)
	{
		std::cout << item << std::endl;
	}


	std::cout << std::endl;
	std::cout << "Graph unique edges count:" << g.countUniqueEdges() << std::endl;
	std::cout << std::endl;


	std::unordered_map<int, int> recommendedListAndRefCount = g.recommendFriendsAndRefCount(target);
	std::cout << "Friend recommendations for user [target] with mutual friends count: " << std::endl;
	for (const auto &pair: recommendedListAndRefCount)
	{
		std::cout << "Friend: " << pair.first << ", Mutual Friends: " << pair.second << std::endl;
	}

	return 0;
}
