#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include <queue>

namespace demo
{
// protocol input struct
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
	EventType event_type{EventType::AttachRequest};

	std::optional<uint32_t> enodeb_id;
	std::optional<uint32_t> mme_id;
	std::optional<uint64_t> imsi;
	std::optional<uint32_t> m_tmsi;
	std::optional<std::vector<uint8_t>> cgi;
};

// protocol handler outputs struct
struct S1apOut
{
	enum S1apOutType
	{
		  Reg
		, UnReg
		, Cgi
	};

	S1apOutType s1ap_type{S1apOutType::Cgi};
	uint64_t imsi{0ull};

	std::vector<uint8_t> cgi;

	friend auto operator== (const S1apOut& aLeft, const S1apOut& aRight) -> bool;
};

class S1apDb
{
	// subscriber status struct
	struct Subscriber
	{
		uint64_t lastActiveTimestamp{0ull};  // last action since EPOCH in ms
		uint32_t enodeb_id{0};               // eNodeB ID
		uint32_t mme_id{0};                  // MME ID
		uint32_t m_tmsi{0};                  // temporary ID
		std::vector<uint8_t> cgi;            // CGI - Payload ?

		// timeout (1 sec) condition flags
		bool waitingForIdentityResponse{false};
		bool waitingForAttachAccept{false};
		bool waitingForRequestAcknowledge{false};
		bool waitingForReleaseResponse{false};
	};

private:
	using imsi = uint64_t;
	using timestamp = uint64_t;
	using m_tmsi = uint32_t;
	using enodeb_id = uint32_t;
	using mme_id = uint32_t;

	std::unordered_map<imsi, Subscriber> m_subscribers;
	std::unordered_map<m_tmsi, imsi> m_m_tmsi2imsi;
	std::unordered_map<enodeb_id, imsi> m_enodeb_id2imsi;
	std::unordered_map<mme_id, imsi> m_mme_id2imsi;

	using TimeToImsi = std::pair<timestamp, imsi>;
	struct ComparePriority
	{
		auto operator()(const TimeToImsi& aA, const TimeToImsi& aB) const -> bool
		{
			// Note: reverse order for min-heap
			return aA.first > aB.first;
		}
	};
	std::priority_queue<TimeToImsi, std::vector<TimeToImsi>, ComparePriority> m_timeStampQueue;

private:
	auto cleanupOldRecords(uint64_t current_time) -> void;

	auto handleAttachRequest(const Event& aEvent) -> std::optional<S1apOut>;
	auto handleAttachRequest_imsi(const Event& aEvent) -> std::optional<S1apOut>;
	auto handleAttachRequest_m_tmsi(const Event& aEvent) -> std::optional<S1apOut>;

	auto handleIdentityResponse(const Event& aEvent) -> std::optional<S1apOut>;
	auto handleAttachAccept(const Event& aEvent) -> std::optional<S1apOut>;

	auto handlePaging(const Event& aEvent) -> std::optional<S1apOut>;

	auto handlePathSwitchRequest(const Event& aEvent) -> std::optional<S1apOut>;
	auto handlePathSwitchRequestAcknowledge(const Event& aEvent)->std::optional<S1apOut>;

	auto handleUEContextReleaseCommand(const Event& aEvent) -> std::optional<S1apOut>;
	auto handleUEContextReleaseResponse(const Event& aEvent) -> std::optional<S1apOut>;

public:
	auto handler(const Event& aEvent) -> std::optional<S1apOut>;
};

} // namespace demo