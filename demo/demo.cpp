#include <iostream>

#include "demo.h"
#include "S1apDb.h"

using namespace std;

int main()
{
	demo::S1apDb foo;
	demo::Event event;
	const auto res = foo.handler(event);

	cout << "Hello CMake." << endl;
	return 0;
}
