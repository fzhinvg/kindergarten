//
// Created by fzhinvg on 2025/6/22.
//
#include "kit.h"

#pragma region easier
// 基类模板
template<typename T>
class Animal {
public:
    void speak() {
        static_cast<T *>(this)->makeSound(); // 编译期绑定派生类方法
    }
};

// 派生类 Dog
class Dog : public Animal<Dog> {
public:
    void makeSound() {
        std::cout << "Woof!" << std::endl;
    }
};

// 派生类 Cat
class Cat : public Animal<Cat> {
public:
    void makeSound() {
        std::cout << "Meow!" << std::endl;
    }
};

#pragma endregion

template<typename Derived>
class IBase {
public:
    void interface() {
        static_cast<Derived *>(this)->implement(); // 编译时的函数重定向
    }
};

class ImplClassA : public IBase<ImplClassA> {
private:
    void implement() {
        std::print("{0} implement\n", typeid(*this).name());
    }

public:
    friend class IBase<ImplClassA>; // 如果要声明为private
};

class ImplClassB : public IBase<ImplClassB> {
private:
    void implement() {
        std::print("{0} implement\n", typeid(*this).name());
    }

public:
    friend class IBase<ImplClassB>;
};


int main() {
#pragma region easier
    Dog dog;
    Cat cat;

    dog.speak(); // Woof!
    cat.speak(); // Meow!

    // 通过基类接口调用（无需虚函数）
    Animal<Dog> &animal1 = dog;
    Animal<Cat> &animal2 = cat;
    animal1.speak(); // Woof!
    animal2.speak(); // Meow!
#pragma endregion

    ImplClassA impl_a;
    ImplClassB impl_b;

    IBase<ImplClassA> &ref_impl_a = impl_a;
    IBase<ImplClassB> &ref_impl_b = impl_b;

    tc::printFunctionSignature(ref_impl_a);
    tc::printFunctionSignature(ref_impl_b);

    ref_impl_a.interface();
    ref_impl_b.interface(); // 通过基类引用调用


    return 0;
}

