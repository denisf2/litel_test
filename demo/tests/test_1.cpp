#include <gtest/gtest.h>
#include "..\S1apDb.h"

//class MyTest : public ::testing::Test
//{
//public:
//protected:
//	static void SetUpTestCase()
//	{
//		std::cout << "TestSuiteSetup" << std::endl;
//	}
//
//	static void TearDownTestCase()
//	{
//
//	}
//};

demo::S1apDb foo;

TEST(ExampleTest, BasicAssertions)
{
	demo::Event event {
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 0,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	const auto res = foo.handler(event);

	const demo::S1apOut out {
				.s1ap_type = demo::S1apOut::Reg,
				.imsi = 100ull,
				.cgi = {{0, 1, 2, 3}}
	};

	EXPECT_EQ(res.value(), out);
}