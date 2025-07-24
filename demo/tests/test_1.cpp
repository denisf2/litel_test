#include <gtest/gtest.h>
#include "..\S1apDb.h"

TEST(Registration_Test, eNodeB_AttachRequest_imsi)
{
	demo::Event event {
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 0,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	const auto res = foo.handler(event);

	const demo::S1apOut out {
				.s1ap_type = demo::S1apOut::Reg,
				.imsi = 100ull,
				.cgi = {{0, 1, 2, 3}}
	};

	EXPECT_EQ(res.value(), out);
}

TEST(Registration_Test, eNodeB_AttachRequest_imsi_same_message)
{
	demo::Event event{
			.timestamp = 10ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 0,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Registration_Test, eNodeB_AttachRequest_new_m_tmsi_new_subs_no_history)
{
	demo::Event event{
			.timestamp = 10ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 0,
			.m_tmsi = 100,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Registration_Test, eNodeB_AttachRequest_old_m_tmsi_old_subs_with_history)
{
	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	event = demo::Event {
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event {
		.timestamp = 1000ull,
		.event_type = demo::Event::EventType::AttachRequest,
		.enodeb_id = 20,
		.m_tmsi = 100,
		.cgi = {{0, 1, 2, 3}}
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Registration_Test, eNodeB_AttachAccept_imsi_new_subs_less_1_sec)
{
	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	const auto res = foo.handler(event);

	const demo::S1apOut out{
				.s1ap_type = demo::S1apOut::Reg,
				.imsi = 100ull,
				.cgi = {{0, 1, 2, 3}}
	};

	EXPECT_EQ(res.value(), out);
}

TEST(Registration_Test, eNodeB_AttachAccept_imsi_new_subs_more_1_sec)
{
	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	event = demo::Event{
			.timestamp = 1100ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Registration_Test, eNodeB_AttachRequest_Identity_Respond_less_1_sec)
{
	demo::Event event{
		.timestamp = 0ull,
		.event_type = demo::Event::EventType::AttachRequest,
		.enodeb_id = 10,
		.imsi = 100ull,
		.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event {
			.timestamp = 920ull,
			.event_type = demo::Event::EventType::IdentityResponse,
			.enodeb_id = 0,
			.mme_id = 0,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	const auto res = foo.handler(event);

	const demo::S1apOut out{
			.s1ap_type = demo::S1apOut::Cgi,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};

	EXPECT_EQ(res.value(), out);
}

TEST(Registration_Test, eNodeB_AttachRequest_Identity_Respond_more_1_sec)
{
	demo::Event event{
		.timestamp = 0ull,
		.event_type = demo::Event::EventType::AttachRequest,
		.enodeb_id = 10,
		.imsi = 100ull,
		.cgi = {{0, 1, 2, 3}}
	};

	demo::S1apDb foo;
	foo.handler(event);
	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 1020ull,
			.event_type = demo::Event::EventType::IdentityResponse,
			.enodeb_id = 0,
			.mme_id = 0,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Deregistration_Test, eNodeB_UEContextReleaseCommand)
{
	demo::S1apDb foo;

	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2000ull,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};

	const auto res = foo.handler(event);

	const demo::S1apOut out{
				.s1ap_type = demo::S1apOut::Cgi,
				.imsi = 100ull,
				.cgi = {{4, 5, 6, 7}}
	};

	EXPECT_EQ(res.value(), out);
}

TEST(Deregistration_Test, eNodeB_UEContextReleaseCommand_not_valid_enode_id)
{
	demo::S1apDb foo;

	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2000ull,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 20,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};

	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Deregistration_Test, eNodeB_UEContextReleaseResponse_less_1_sec)
{
	demo::S1apDb foo;

	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2000ull,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2500ull,
			.event_type = demo::Event::EventType::UEContextReleaseResponse,
			.enodeb_id = 10,
			.mme_id = 10,
	};
	const auto res = foo.handler(event);

	const demo::S1apOut out{
				.s1ap_type = demo::S1apOut::UnReg,
				.imsi = 100ull,
				.cgi = {{4, 5, 6, 7}}
	};

	EXPECT_EQ(res.value(), out);
}

TEST(Deregistration_Test, eNodeB_UEContextReleaseResponse_less_1_sec_not_valid_enodeb_id)
{
	demo::S1apDb foo;

	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2000ull,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2500ull,
			.event_type = demo::Event::EventType::UEContextReleaseResponse,
			.enodeb_id = 20,
			.mme_id = 10,
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Deregistration_Test, eNodeB_UEContextReleaseResponse_more_1_sec)
{
	demo::S1apDb foo;

	demo::Event event{
			.timestamp = 0ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 10,
			.imsi = 100ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 900ull,
			.event_type = demo::Event::EventType::AttachAccept,
			.enodeb_id = 10,
			.mme_id = 10,
			.m_tmsi = 100,
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2000ull,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 3500ull,
			.event_type = demo::Event::EventType::UEContextReleaseResponse,
			.enodeb_id = 10,
			.mme_id = 10,
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

// TODO: TEST(PathChanging_Test, eNodeB_AttachRequest_Identity_Respond)
// TODO: TEST(Timeouts_Test, eNodeB_AttachRequest_Identity_Respond)
// TODO: TEST(CallFlow_Test, eNodeB_AttachRequest_Identity_Respond)