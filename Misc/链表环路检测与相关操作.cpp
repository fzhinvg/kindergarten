//
// Created by fzhinvg on 2024/12/9.
//
#include <iostream>
#include <unordered_set>
#include <list>
#include <stack>
#include <ctime>
#include <cstdlib>

// 具体使用哪种解决方案还得看问题规模和对象

//从list的结构设计来看，如果一个list有环路说明一定是死循环
struct ListNode
{
	int val;
	ListNode *next;

	explicit ListNode(int x) : val(x), next(nullptr)
	{}

	~ListNode()
	{
//		std::cout << "Destructor called for value: " << this->val << std::endl;
	}
};

// 我爱copilot
ListNode *generateCycleList(int length)
{
	if (length <= 0)
	{
		return nullptr;
	}

	// 初始化链表头节点
	ListNode *head = new ListNode(1);
	ListNode *current = head;

	// 生成链表节点
	for (int i = 2; i <= length; ++i)
	{
		current->next = new ListNode(i);
		current = current->next;
	}

	// 随机选择一个位置创建环路
	std::srand(std::time(nullptr)); // 使用当前时间作为随机种子
	int cycle_pos = std::rand() % length + 1;

	// 找到环路节点
	ListNode *cycle_node = head;
	for (int i = 1; i < cycle_pos; ++i)
	{
		cycle_node = cycle_node->next;
	}

	// 创建环路
	current->next = cycle_node;

	std::cout << "Cycle created at node with value: " << cycle_node->val << std::endl;
	return head;
}

// 使用快慢指针检测链表中的环路
bool hasCycleFloyd(ListNode *head)
{
	if (head == nullptr || head->next == nullptr)
	{
		return false;
	}

	ListNode *slow = head;
	ListNode *fast = head;

	while (fast != nullptr && fast->next != nullptr)
	{
		slow = slow->next;         // 慢指针每次移动一步
		fast = fast->next->next;   // 快指针每次移动两步

		if (slow == fast)
		{
			return true;           // 如果快慢指针相遇，说明存在环
		}
	}

	return false;  // 如果快指针到达链表末尾，说明不存在环
}

// 使用unordered_set检测链表中的环路
bool hasCycleSet(ListNode *head)
{
	std::unordered_set<ListNode *> nodeSet;

	ListNode *current = head;
	while (current != nullptr)
	{
		// 如果当前节点已在集合中，说明存在环路
		if (nodeSet.find(current) != nodeSet.end())
		{
			return true;
		}
		// 将当前节点加入集合
		nodeSet.insert(current);
		current = current->next;
	}

	return false;  // 遍历完整个链表，没有发现环路
}

void deleteNodes(ListNode *node)
{
	if (hasCycleFloyd(node))
	{
		std::cerr << "cant delete" << std::endl;
		return;
	}

	if (node->next == nullptr)
	{
		return;
	}

	// 递归删除下一个节点
	deleteNodes(node->next);

	// 删除当前节点
	std::cout << "Deleting node with value: " << node->val << std::endl;
	delete node->next;

}

// 使用栈和递归来解决释放问题好像看起来很合理，但是如果有环路的话就需要另一套逻辑
void deleteNodesUsingStack(ListNode *head)
{
	std::stack<ListNode *> nodeStack{};

	// 将链表中的每个节点推进栈中
	ListNode *current = head;
	while (current != nullptr)
	{
		nodeStack.push(current);
		current = current->next;
	}

	// 从栈中逐一弹出并删除节点
	while (!nodeStack.empty())
	{
		ListNode *node = nodeStack.top();
		nodeStack.pop();
		std::cout << "Deleting node with value: " << node->val << std::endl;
		delete node;
	}
}

// 我目前想到两套解决方案
// 如果有环，得找到环路入口，断开环路，很合理
void disposeCycleList(ListNode *head)
{
	if (head == nullptr)
	{
		return;
	}

	ListNode *slow = head;
	ListNode *fast = head;

	// 检测环路
	while (fast != nullptr && fast->next != nullptr) // 防止越界
	{
		slow = slow->next;
		fast = fast->next->next;

		if (slow == fast) // 有环
		{
			ListNode *entry = head;
			std::cout << "Find collision with value: " << slow->val << std::endl;
			while (entry != slow)
			{
				entry = entry->next;
				slow = slow->next;
			}// 对于数学能力堪忧的我，这里的相同步进但是能在正确的entry相遇，简直是魔法
			std::cout << "Find entry with value: " << entry->val << std::endl;

			// 找到入口前一个，就是需要断开的地方
			ListNode *entry_pre = entry; // 用这个初始化可以少走几格，逻辑上比head强点
			while (entry_pre->next != entry)
			{
				entry_pre = entry_pre->next;
			}
			std::cout << "Find entry_pre with value: " << entry_pre->val << std::endl;
			entry_pre->next = nullptr; // 断开
			break; // 合理！
		}
	}
//	deleteNodes(head);
	deleteNodesUsingStack(head);
}

// unordered_set的对于这个需求的性能看起来比rb-tree的set强点
// 而且逻辑简单，不过上面断开环的部分也不是一无所处，还可以单独拉出来作为一个断开的method
// 这个方法的适用性更广，无论是否存在环路都可以用这个，理论上
void deleteNodesWithCycle(ListNode *head)
{
	std::unordered_set<ListNode *> visited;

	ListNode *current = head;
	while (current != nullptr)
	{
		// 如果当前节点已经在集合中，说明存在环，而且重复了，就break
		if (visited.find(current) != visited.end())
		{
			break;
		}
		// 将当前节点加入集合
		visited.insert(current);
		ListNode *temp = current;
		current = current->next;
		std::cout << "Deleting node with value: " << temp->val << std::endl;
		delete temp;
	}
}

int main()
{
	auto *head = new ListNode(1); // 命名为head有点学院派且就地解释不管后续理解性的意味，不过不太重要也能接受
	head->next = new ListNode(2);
	head->next->next = new ListNode(3);
	head->next->next->next = new ListNode(4);
//	head->next->next->next->next = new ListNode(5);
	head->next->next->next->next = head->next;
//	head->next->next->next->next->next = head->next; // 创建环

//	auto *list = new ListNode(0);
//	list->next = new ListNode(1);
//	list->next->next = new ListNode(2);
//	list->next->next->next = new ListNode(3);

	// 快慢指针方法
	if (hasCycleFloyd(head))
	{
		std::cout << "Floyd Cycle" << std::endl;
	} else
	{
		std::cout << "Floyd NoCycle" << std::endl;
	}

	// unordered_set方法
	if (hasCycleSet(head))
	{
		std::cout << "unordered_set Cycle" << std::endl;
	} else
	{
		std::cout << "unordered_set NoCycle " << std::endl;
	}


	int break_point = 0;
	for (auto item = head; item != nullptr; item = item->next)
	{
		std::cout << item->val;
		if (break_point == 50)
		{
			break;
		}
		++break_point;
	}
	std::cout << std::endl;


	disposeCycleList(head);


//	deleteNodesUsingStack(list);
//	deleteNodes(list);
//	deleteNodes(head);

	auto random_list = generateCycleList(20);
	disposeCycleList(random_list);
	auto random_list1 = generateCycleList(20);
	deleteNodesWithCycle(random_list1);

	return 0;
}
// 上面的方法应该适用于双向链表结构
// 我现在有一个未经验证的直观感觉，循环的圈只可能出现在尾部，不可能是战神诸神黄昏的Ωlogo一样的中间位置。