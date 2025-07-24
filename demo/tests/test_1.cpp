#include <gtest/gtest.h>
#include "..\S1apDb.h"

constexpr uint64_t session_timeout_24_hours_in_ms{24ull * 60 * 60 * 1000};

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

TEST(PathChanging_Test, eNodeB_PathSwitchRequest)
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
			.event_type = demo::Event::EventType::PathSwitchRequest,
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

TEST(PathChanging_Test, eNodeB_PathSwitchRequest_new_enode_id)
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
			.event_type = demo::Event::EventType::PathSwitchRequest,
			.enodeb_id = 20,
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

TEST(PathChanging_Test, eNodeB_PathSwitchRequest_new_enode_id_not_valid_mme_id)
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
			.event_type = demo::Event::EventType::PathSwitchRequest,
			.enodeb_id = 20,
			.mme_id = 20,
			.cgi = {{4, 5, 6, 7}}
	};

	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(PathChanging_Test, eNodeB_PathSwitchRequestAcknowledge_less_1_sec)
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
			.event_type = demo::Event::EventType::PathSwitchRequest,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 2500ull,
			.event_type = demo::Event::EventType::PathSwitchRequestAcknowledge,
			.enodeb_id = 10,
			.mme_id = 10,
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(PathChanging_Test, eNodeB_PathSwitchRequestAcknowledge_more_1_sec)
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
			.event_type = demo::Event::EventType::PathSwitchRequest,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = 3500ull,
			.event_type = demo::Event::EventType::PathSwitchRequestAcknowledge,
			.enodeb_id = 10,
			.mme_id = 10,
	};
	const auto res = foo.handler(event);

	EXPECT_EQ(res, std::nullopt);
}

TEST(Timeouts_Test, eNodeB_subscriber_is_not_active_exectly_24_hours_is_valid)
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
			.timestamp = 1000ull,
			.event_type = demo::Event::EventType::AttachRequest,
			.enodeb_id = 20,
			.imsi = 200ull,
			.cgi = {{0, 1, 2, 3}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = session_timeout_24_hours_in_ms,
			.event_type = demo::Event::EventType::UEContextReleaseCommand,
			.enodeb_id = 10,
			.mme_id = 10,
			.cgi = {{4, 5, 6, 7}}
	};
	foo.handler(event);

	event = demo::Event{
			.timestamp = session_timeout_24_hours_in_ms + 1,
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

 TEST(Timeouts_Test, eNodeB_subscriber_is_not_active_more_24_hours)
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

	 event = demo::Event {
			 .timestamp = session_timeout_24_hours_in_ms + 1,
			 .event_type = demo::Event::EventType::AttachRequest,
			 .enodeb_id = 20,
			 .imsi = 200ull,
			 .cgi = {{0, 1, 2, 3}}
	 };
	 foo.handler(event);

	 event = demo::Event{
			 .timestamp = session_timeout_24_hours_in_ms + 2000,
			 .event_type = demo::Event::EventType::UEContextReleaseCommand,
			 .enodeb_id = 10,
			 .mme_id = 10,
			 .cgi = {{4, 5, 6, 7}}
	 };
	 foo.handler(event);

	 event = demo::Event{
			 .timestamp = session_timeout_24_hours_in_ms + 2500,
			 .event_type = demo::Event::EventType::UEContextReleaseResponse,
			 .enodeb_id = 10,
			 .mme_id = 10,
	 };
	 const auto res = foo.handler(event);

	 EXPECT_EQ(res, std::nullopt);
 }

 TEST(CallFlow_Test, eNodeB_all_messages_from_not_accepted)
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
			 .timestamp = 10ull,
			 .event_type = demo::Event::EventType::Paging,
			 .m_tmsi = 10,
			 .cgi = {{2, 3, 4, 5}},
	 };
	 auto res = foo.handler(event);
	 EXPECT_EQ(res, std::nullopt);

	 event = demo::Event{
			 .timestamp = 21ull,
			 .event_type = demo::Event::EventType::PathSwitchRequest,
			 .enodeb_id = 10,
			 .mme_id = 10,
			 .cgi = {{0, 1, 2, 3}},
	 };
	 res = foo.handler(event);
	 EXPECT_EQ(res, std::nullopt);

	 event = demo::Event{
			 .timestamp = 31ull,
			 .event_type = demo::Event::EventType::UEContextReleaseCommand,
			 .enodeb_id = 10,
			 .mme_id = 10,
			 .cgi = {{0, 1, 2, 3}},
	 };
	 res = foo.handler(event);

	 const demo::S1apOut out{
			 .s1ap_type = demo::S1apOut::Cgi,
			 .imsi = 100ull,
			 .cgi = {{0, 1, 2, 3}}
	 };

	 EXPECT_EQ(res.value(), out);
 }

 TEST(CallFlow_Test, all_messages_two_enodebs_unreg_and_restore_one)
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
			 .timestamp = 0ull,
			 .event_type = demo::Event::EventType::AttachRequest,
			 .enodeb_id = 20,
			 .imsi = 200ull,
			 .cgi = {{4, 5, 6, 7}}
	 };
	 foo.handler(event);

	 event = demo::Event{
			 .timestamp = 100ull,
			 .event_type = demo::Event::EventType::AttachAccept,
			 .enodeb_id = 20,
			 .mme_id = 20,
			 .m_tmsi = 20
	 };
	 auto res = foo.handler(event);
	 auto out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Reg,
		 .imsi = 200,
		 .cgi = {{4, 5, 6, 7}}
	 };
	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 300ull,
			 .event_type = demo::Event::EventType::AttachAccept,
			 .enodeb_id = 10,
			 .mme_id = 10,
			 .m_tmsi = 10
	 };
	 res = foo.handler(event);
	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Reg,
		 .imsi = 100,
		 .cgi = {{0, 1, 2, 3}}
	 };
	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 301ull,
			 .event_type = demo::Event::EventType::Paging,
			 .m_tmsi = 10,
			 .cgi = {{2, 3, 4, 5}},
	 };
	 res = foo.handler(event);

	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Cgi,
		 .imsi = 100,
		 .cgi = {{2, 3, 4, 5}}
	 };
	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 401ull,
			 .event_type = demo::Event::EventType::PathSwitchRequest,
			 .enodeb_id = 10,
			 .mme_id = 10,
			 .cgi = {{0, 1, 2, 5}},
	 };
	 res = foo.handler(event);
	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Cgi,
		 .imsi = 100,
		 .cgi = {{0, 1, 2, 5}}
	 };
	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 601ull,
			 .event_type = demo::Event::EventType::PathSwitchRequestAcknowledge,
			 .enodeb_id = 10,
			 .mme_id = 10,
	 };
	 res = foo.handler(event);
	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Cgi,
		 .imsi = 100,
		 .cgi = {{0, 1, 2, 5}}
	 };
	 EXPECT_EQ(res, std::nullopt);

	 event = demo::Event{
			 .timestamp = 701ull,
			 .event_type = demo::Event::EventType::UEContextReleaseCommand,
			 .enodeb_id = 20,
			 .mme_id = 20,
			 .cgi = {{0, 1, 2, 3}},
	 };
	 res = foo.handler(event);

	 out = demo::S1apOut {
			 .s1ap_type = demo::S1apOut::S1apOutType::Cgi,
			 .imsi = 200ull,
			 .cgi = {{0, 1, 2, 3}}
	 };

	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 701ull,
			 .event_type = demo::Event::EventType::UEContextReleaseResponse,
			 .enodeb_id = 20,
			 .mme_id = 20,
	 };
	 res = foo.handler(event);

	 out = demo::S1apOut{
			 .s1ap_type = demo::S1apOut::S1apOutType::UnReg,
			 .imsi = 200ull,
			 .cgi = {{0, 1, 2, 3}}
	 };

	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 800ull,
			 .event_type = demo::Event::EventType::AttachRequest,
			 .enodeb_id = 20,
			 .m_tmsi = 20,
			 .cgi = {{4, 5, 6, 7}}
	 };
	 res = foo.handler(event);

	 EXPECT_EQ(res, std::nullopt);

	 event = demo::Event{
			 .timestamp = 900ull,
			 .event_type = demo::Event::EventType::AttachAccept,
			 .enodeb_id = 20,
			 .mme_id = 20,
			 .m_tmsi = 30
	 };
	 res = foo.handler(event);
	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Reg,
		 .imsi = 200,
		 .cgi = {{4, 5, 6, 7}}
	 };
	 EXPECT_EQ(res.value(), out);

	 event = demo::Event{
			 .timestamp = 1000ull,
			 .event_type = demo::Event::EventType::IdentityResponse,
			 .enodeb_id = 20,
			 .mme_id = 20,
			 .imsi = 200,
			 .cgi = {{5, 5, 5, 5}}
	 };
	 res = foo.handler(event);
	 out = demo::S1apOut{
		 .s1ap_type = demo::S1apOut::S1apOutType::Cgi,
		 .imsi = 200,
		 .cgi = {{5, 5, 5, 5}}
	 };
	 EXPECT_EQ(res.value(), out);
}