#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

namespace demo
{
struct Event
{
	enum class EventType
	{
		  AttachRequest                  // ENODEB --> MME
		, IdentityResponse               // ENODEB --> MME
		, AttachAccept                   // MME --> ENODEB
		, Paging                         // ENODEB --> MME
		, PathSwitchRequest              // ENODEB --> MME
		, PathSwitchRequestAcknowledge   // MME -- > ENODEB
		, UEContextReleaseCommand        // ENODEB --> MME
		, UEContextReleaseResponse       // MME -- > ENODEB
	};

	// sizeof(unsigned long) == [4|8] depends on platform
	uint64_t timestamp{0ul}; // since EPOCH in ms
	EventType event_type;

	std::optional<uint32_t> enodeb_id;
	std::optional<uint32_t> mme_id;
	std::optional<uint64_t> imsi;
	std::optional<uint32_t> m_tmsi;
	std::optional<std::vector<uint8_t>> cgi;
};

struct S1apOut
{
	enum S1apOutType
	{
		  Reg
		, UnReg
		, Cgi
	};

	S1apOutType s1ap_type;
	uint64_t imsi{0ul};

	std::vector<uint8_t> cgi;
};

class S1apDb
{
	struct Subscriber
	{
		uint64_t lastActiveTimestamp{0ul};   // last action since EPOCH in ms
		uint32_t enodeb_id{0};               // eNodeB ID
		uint32_t mme_id{0};                  // MME ID
		uint32_t m_tmsi{0};                  // temporary ID
		std::vector<uint8_t> cgi;            // CDI - Payload ?
	};

private:
	using imsi = uint64_t;

	std::unordered_map<imsi, Subscriber> m_subscribers;
	// TODO: make search Subscriber(enodeb_id)
	// TODO: make search Subscriber(mme_id)
	// TODO: make search Subscriber(m_tmsi)
	// TODO: check timeout somehow and make cleanup

private:
	auto handleAttachRequest(const Event& aEvent) -> std::optional<S1apOut>;

public:
	auto handler(const Event& aEvent) -> std::optional<S1apOut>;
};

} // namespace demo