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


	uint64_t timestamp{0ul};   // since EPOCH in ms
	EventType event_type;

	std::optional<uint32_t> enodeb_id; // optional
	std::optional<uint32_t> mme_id; // optional
	std::optional<uint64_t> imsi; // optional
	std::optional<uint32_t> m_tmsi; //optional
	std::optional<std::vector<uint8_t>> cgi; // optional
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
	unsigned long imsi{0ul};

	std::vector<unsigned char> cgi;
};

class S1apDb
{
	class Subscriber
	{
	private:
		uint64_t lastActiveTimestamp{0ul};   // last action since EPOCH in ms
		uint32_t enodeb_id;                  // eNodeB ID
		uint32_t mme_id;                     // MME ID
		uint32_t m_tmsi;                     // temporary ID
		std::vector<uint8_t> cgi;            // CDI - Payload ?

	public:
	};

private:
	std::unordered_map<uint64_t, Subscriber> m_subscribers;

public:
	auto handler(const Event& aEvent) -> std::optional<S1apOut>;
};

} // namespace demo