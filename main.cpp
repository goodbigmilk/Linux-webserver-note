#include <iostream>

class A
{
public:
	int a;
	int* b;
public:
	A() :a(0), b(nullptr) {};
};

int main()
{
	A ab;
	A* sd = &ab;
	std::cout << sd->a << "  " << sd->b;
	//delete sd;
	std::cout << sd->a << "  " << sd->b;
	return 0;
}