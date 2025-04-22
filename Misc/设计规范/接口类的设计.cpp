//
// Created by fzhinvg on 2025/4/22.
//
#include <iostream>
#include <string>
#include <cmath>
#include <memory>

#pragma region interface_class_design

class IShape
{
public:
    virtual ~IShape() = 0;  // 声明为纯虚析构函数
    [[nodiscard]] virtual double area() const = 0;  // 纯虚函数
    virtual void scale(double factor) = 0;  // 纯虚函数
};

IShape::~IShape() = default;  // 接口类可以生成对象这件事不太对，因此声明为纯虚并提供一个空实现

#pragma endregion

class Circle : public IShape
{
public:
    explicit Circle(double radius) : _radius(radius)
    {}

    [[nodiscard]] double area() const override
    {
        return M_PI * _radius * _radius;
    }

    void scale(double factor) override
    {
        _radius *= factor;
    }

    [[nodiscard]] double radius() const
    { return _radius; }  // 公有接口

private:
    double _radius;
};


// 对称操作符重载：非成员函数（处理 double + Circle）定义为非成员函数的目的是为了支持隐式类型转换的对称性
// 对称是数学上的概念，指的是满足交换律的二元操作符
Circle operator+(double r, const Circle &c)
{
    return Circle(c.radius() + r);
}
Circle operator+(const Circle &c, double r)
{
    return Circle(c.radius() + r);
}
// 写代码的第一要义就是能动，不要因为各种XX开销就畏首畏尾，优化是三步曲的最后一步，使用公共getter方法在O3下会被优化
// 我尝试寻找简化写法，不然显得冗余，但是实际上这是一种非常好的写法，除了比较繁琐，但是它直观准确，易于调试
// 还有一点，慎用宏，我并不是一个喜欢用宏的人，我也不提倡别人使用宏定义来实现意义不大的且所谓的“便利与极简”

/*
// 模板化对称操作符
template <typename T>
auto operator+(const Circle& c, T add)
-> std::enable_if_t<std::is_arithmetic_v<T>, Circle> {
    return Circle(c.radius() + add);
}

template <typename T>
auto operator+(T add, const Circle& c)
-> std::enable_if_t<std::is_arithmetic_v<T>, Circle> {
    return c + add; // 复用第一个版本
}
*/

// 如果没有直接访问类私有成员的需求，且仅通过类的公有接口（公有成员函数或公有数据成员）就能实现功能
// 那么将函数定义为非成员函数通常是更好的设计选择
// 这背后的核心思想是最小化封装破坏和提高代码的灵活性
namespace ShapeUtils
{
    // 通过公有接口访问，无需友元或私有成员访问，使用友元会破坏封装性，与我们的初衷背道而驰
    void printArea(const IShape &shape)
    {
        std::cout << "Area: " << shape.area() << std::endl;
    }

    // 模板工具函数
    template<typename T>
    T clamp(T value, T min, T max)
    {
        return (value < min) ? min : (value > max) ? max : value;
    }
}

namespace StringUtils
{
    std::string trimmed(const std::string &s)
    {
        size_t start = s.find_first_not_of(' ');
        size_t end = s.find_last_not_of(' ');
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
}
// 不使用c# java中类似的静态类的原因：
// 1.冗余的访问控制，在静态类中还要多余使用public声明公开
// 但是这些函数理应是公开的，因为他们使用公共接口不直接操作和访问其它类型的私有成员
// 2.不必要的类语义负担，类的设计通常隐含了某种“实体”的抽象，而工具函数集合是无状态的、算法性的
// 3.无法分散定义，类的静态成员函数必须在类定义内部或外部统一实现，而命名空间允许在多个文件中扩展，方便模块化管理
// 4.命名空间的天然优势，命名空间可以将一组相关函数、类型、常量等逻辑上归类，无需强制塞入类的静态成员中
// 支持分散在多个文件中的同名命名空间自动合并，便于大型项目的模块化管理。


// ====================== 主函数演示 ======================
int main()
{
    // 1. 接口多态性演示 多态性依赖于virtual函数，此时成员函数是必要的
    std::unique_ptr<IShape> shape = std::make_unique<Circle>(5.0); // 我们应该避免使用“裸指针”，应该优先考虑构造器和析构器共同作用和实现的RAII
    ShapeUtils::printArea(*shape);  // Area: 78.5398

    // 2. 对称操作符重载演示
    Circle c1 = Circle(3.0) + 2.0;
    Circle c2 = 2.0 + Circle(3.0);
    std::cout << "c1 radius: " << c1.radius() << std::endl;  // 5
    std::cout << "c2 radius: " << c2.radius() << std::endl;  // 5

    // 3. 工具函数使用演示
    std::string s = "   Hello World   ";
    std::cout << "Trimmed: |" << StringUtils::trimmed(s) << "|" << std::endl;  // |Hello World|

    int clamped = ShapeUtils::clamp(15, 0, 10);
    std::cout << "Clamped value: " << clamped << std::endl;  // 10

    // 4. 验证虚析构函数（通过valgrind等工具检查无内存泄漏）
    IShape *ptr = new Circle(2.0);
    delete ptr;  // 正确调用派生类析构函数

    return 0;
}