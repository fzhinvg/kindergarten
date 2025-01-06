//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>

class Animal
{
public:
	virtual void makeSound()
	{
		std::cout << "Some generic animal sound" << std::endl;
	}

};

class Dog : public Animal
{
public:
	void makeSound() override
	{
		std::cout << "Bark" << std::endl;
	}
};

class Cat : public Animal
{
public:
	void makeSound() override
	{
		std::cout << "Meow" << std::endl;
	}
};

void animalSound_r(Animal &animal)
{
	animal.makeSound();
}

void animalSound(Animal animal)
{
	animal.makeSound();
}

int main()
{
	Dog dog;
	Cat cat;

	animalSound_r(dog); // 输出: Bark
	animalSound_r(cat); // 输出: Meow

	auto p = static_cast<Animal>(dog);
	Animal *bp = &dog;
	auto vptr = reinterpret_cast<uintptr_t **>(bp);
	uintptr_t *vtable = *vptr;
	using vfunc_t = void (*)();
	auto vfunc_addr = vtable[0];
	vfunc_t vfunc = reinterpret_cast<vfunc_t>(vfunc_addr);
	vfunc();
//	std::cout << typeid(dog).name() << std::endl;
//	std::cout << typeid(cat).name() << std::endl;
//	std::cout << typeid(*bp).name() << std::endl;
	std::cout << typeid(bp).name() << std::endl;
	animalSound(*bp);
	animalSound_r(*bp);
//	bp->makeSound();
//	animalSound_r(*bp);
//	animalSound_r(dog);
//	animalSound(dog);
//	animalSound_r(cat);
//	animalSound(cat);

	return 0;
}

#if 0
#include <iostream>

class Animal
{
public:
	virtual void makeSound() const
	{
		std::cout << "Some generic animal sound" << std::endl;
	}
};

class Dog : public Animal
{
public:
	void makeSound() const override
	{
		std::cout << "Bark" << std::endl;
	}

	void wagTail() const
	{
		std::cout << "Wagging tail" << std::endl;
	}
};

void sliceExample(Animal animal)
{
	animal.makeSound(); // 只会调用基类的 makeSound()
}

void sliceExample_r(Animal &animal)
{
	animal.makeSound();
}

int main()
{
	Dog dog;
	sliceExample(dog); // 发生对象切片，生成了一个animal对象，只包含dog的基类部分
	sliceExample_r(dog);
	return 0;
}

#endif