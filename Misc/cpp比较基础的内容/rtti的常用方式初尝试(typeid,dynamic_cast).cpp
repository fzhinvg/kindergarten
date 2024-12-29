//
// Created by fzhinvg on 2024/12/29.
//
// rtti相关
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <vector>

//class Base
//{
//public:
//	virtual ~Base() = default;
//	/*
//	 * 当使用基类指针或者引用指向派生类对象,并且使用这个指针或者引用来删除对象时
//	 * 如果基类系后期不是虚函数,就只会触发基类析构器,不会调用派生类的析构器
//	 * 导致资源泄漏或其他问题
//	 * 所以,这是一个好的行为,当然我说的是对于多态
//	 */
//};

class Shape
{
public:
	virtual ~Shape() = default;

	virtual void draw() const = 0;
};

class Circle : public Shape
{
public:
	void draw() const override
	{
		std::cout << "Drawing Circle" << std::endl;
	}

	void radius() const
	{
		std::cout << "Circle radius" << std::endl;
	}
};

class Rectangle : public Shape
{
public:
	void draw() const override
	{
		std::cout << "Drawing Rectangle" << std::endl;
	}

	void area() const
	{
		std::cout << "Rectangle area" << std::endl;
	}
};

// type traits 是编译期的工具,不属于 RTTI 的范畴
void typeTraitsDemo() // 为了演示区别
{
	std::cout << std::boolalpha; // console true false
	std::cout << "Is int an integral type? " << std::is_integral<int>::value << std::endl;
	std::cout << "Is float a floating point type? " << std::is_floating_point<float>::value << std::endl;
}

void processShapes(const std::vector<Shape *> &shapes)
{
	for (const auto &shape: shapes)
	{
		shape->draw();

		// rtti 实际运用所在
		if (auto *circle = dynamic_cast<Circle *>(shape))
		{
			circle->radius();
		} else if (auto *rectangle = dynamic_cast<Rectangle *>(shape))
		{
			rectangle->area();
		} else
		{
			std::cout << "Unknown Shape" << std::endl;
		}
	}
}

int main()
{
//	typeTraitsDemo();

	Circle circle;
	Rectangle rectangle;
	Shape *shapes[] = {&circle, &rectangle};
	std::vector<Shape *> shapeVector(std::begin(shapes), std::end(shapes));

	std::cout << "Processing shapes:" << std::endl;
//	processShapes({shapes, shapes + 2}); // 这种方式构造vector就是一个半开区间[a,b)
//	processShapes({shapes, shapes + (sizeof(shapes) / sizeof(shapes[0]))});
	processShapes({std::begin(shapes), std::end(shapes)}); // amazing

	return 0;
}
