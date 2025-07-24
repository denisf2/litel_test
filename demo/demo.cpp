#include <iostream>

#include "demo.h"
#include "S1apDb.h"

using namespace std;

int main()
{
	try
	{
		demo::S1apDb foo;
		demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 0,
			.imsi = 0ull,
			.cgi = {{}}
		};
		const auto res = foo.handler(event);
	}
	catch(std::exception aExc)
	{
		std::cout << aExc.what() << endl;
	}

	cout << "Hello CMake." << endl;
	return 0;
}
