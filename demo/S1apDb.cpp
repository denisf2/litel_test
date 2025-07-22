#include "S1apDb.h"

namespace demo
{

constexpr uint64_t request_timeout_1_sec_ms{1000ull};

auto S1apDb::handler(const Event& aEvent) -> std::optional<S1apOut>
{
	cleanupOldRecords(aEvent.timestamp);

	using ET = Event::EventType;

	switch(aEvent.event_type)
	{
		case ET::AttachRequest:
			return handleAttachRequest(aEvent);
		case ET::IdentityResponse:
			// TODO: return handleIdentityResponse();
		case ET::AttachAccept:
			// TODO: return handleAttachAccept();
		case ET::Paging:
			return handlePaging(aEvent);
		case ET::PathSwitchRequest:
			// TODO: return handlePathSwitchRequest();
		case ET::PathSwitchRequestAcknowledge:
			// TODO: return handlePathSwitchRequestAcknowledge();
		case ET::UEContextReleaseCommand:
			return handleUEContextReleaseCommand(aEvent);
		case ET::UEContextReleaseResponse:
			 return handleUEContextReleaseResponse(aEvent);
		default:
			return std::nullopt;
	}
}

auto S1apDb::cleanupOldRecords(uint64_t aCurrentTime) -> void
{
	constexpr uint64_t session_timeout_24_hours_ms{24ull * 60 * 60 * 1000};

	// check the oldest subscriber is still valid
	// if no remove it and find new the oldest
	if(aCurrentTime - m_theOldestSubscriber.timestamp > session_timeout_24_hours_ms)
	{
		m_subscribers.erase(m_theOldestSubscriber.imsi);
		// TODO: remove subscriber in other containers

		findTheOldestSubscriber();
	}
}

auto S1apDb::findTheOldestSubscriber() -> void
{
	m_theOldestSubscriber.reset();

	for(const auto& [imsi, subscriber] : m_subscribers)
	{
		if(subscriber.lastActiveTimestamp < m_theOldestSubscriber.timestamp)
		{
			m_theOldestSubscriber.timestamp = subscriber.lastActiveTimestamp;
			m_theOldestSubscriber.imsi = imsi;
		}
	}
}

auto S1apDb::handleAttachRequest(const Event& aEvent) -> std::optional<S1apOut>
{
	if(aEvent.imsi.has_value()) // has imsi
	{
		// imsi -> new / update subscriber
		const auto imsi = aEvent.imsi.value();
		// TODO: double run unordered_map lookup. insert, emplace, insert_or_assign overwrites full structure
		const auto newSubscriber = not m_subscribers.contains(imsi);
		auto& subscriber = m_subscribers[imsi];

		subscriber.lastActiveTimestamp = aEvent.timestamp;
		subscriber.enodeb_id = aEvent.enodeb_id.value();
		subscriber.cgi = aEvent.cgi.value();

		// TODO: update all indexes
		m_enodeb_id2imsi[imsi] = imsi;

		if(newSubscriber)
		{
			return {{
					.s1ap_type = S1apOut::S1apOutType::Reg,
					.imsi = imsi,
					.cgi = aEvent.cgi.value()
				}};
		}
	}
	else if(aEvent.m_tmsi.has_value()) // no imsi and has m_tmsi
	{
		// find old imsi and update
		// m_tmsi -> imsi -> new / update subscriber
		if(const auto imsiIter = m_m_tmsi2imsi.find(aEvent.m_tmsi.value()); m_m_tmsi2imsi.cend() != imsiIter)
		{
			const auto& imsi = imsiIter->second;
			// TODO: double run unordered_map lookup. insert, emplace, insert_or_assign overwrites full structure
			const auto newSubscriber = not m_subscribers.contains(imsi);
			auto& subscriber = m_subscribers[imsi];

			subscriber.lastActiveTimestamp = aEvent.timestamp;
			subscriber.enodeb_id = aEvent.enodeb_id.value();
			subscriber.m_tmsi = aEvent.m_tmsi.value();
			subscriber.cgi = aEvent.cgi.value();

			// TODO: update all indexes
			m_m_tmsi2imsi[imsi] = imsi;

			if(newSubscriber)
			{
				return {{
						.s1ap_type = S1apOut::S1apOutType::Reg,
						.imsi = imsi,
						.cgi = aEvent.cgi.value()
					}};
			}
		}
	}

	return std::nullopt;
}


auto S1apDb::handlePaging(const Event& aEvent) -> std::optional<S1apOut>
{
	// Paging{m_tmsi, cgi}

	// m_tmsi -> imsi
	if(const auto imsiIter = m_m_tmsi2imsi.find(aEvent.m_tmsi.value()); m_m_tmsi2imsi.cend() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto subscriber = m_subscribers.find(imsi); m_subscribers.end() != subscriber)
		{
			subscriber->second.lastActiveTimestamp = aEvent.timestamp;
			subscriber->second.cgi = aEvent.cgi.value();

			// TODO: return struct on change cgi or always?
			return {{
						.s1ap_type = S1apOut::S1apOutType::Cgi,
						.imsi = imsi,
						.cgi = aEvent.cgi.value(),
					}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handleUEContextReleaseCommand(const Event& aEvent) -> std::optional<S1apOut>
{
	// UEContextReleaseCommand {enodeb_id_4, mme_id_4, cgi_7}
	// TODO: what should i prefer enodeb_id or mme_id?
	if(const auto imsiIter = m_enodeb_id2imsi.find(aEvent.enodeb_id.value()); m_enodeb_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto subscriber = m_subscribers.find(imsi); m_subscribers.end() != subscriber)
		{
			subscriber->second.lastActiveTimestamp = aEvent.timestamp;
			// TODO: do i need update cgi here?
			subscriber->second.cgi = aEvent.cgi.value();
			subscriber->second.waitingForRequestAcknowledge = true;

			// TODO: return struct on change cgi or always?
			return {{
					.s1ap_type = S1apOut::S1apOutType::Cgi,
					.imsi = imsi,
					.cgi = aEvent.cgi.value()
				}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handleUEContextReleaseResponse(const Event& aEvent) -> std::optional<S1apOut>
{
	// UEContextReleaseResponse {enodeb_id_4, mme_id_4}
	// TODO: what should i prefer enodeb_id or mme_id?
	if(const auto imsiIter = m_enodeb_id2imsi.find(aEvent.enodeb_id.value()); m_enodeb_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto subscriber = m_subscribers.find(imsi); m_subscribers.end() != subscriber)
		{
			// check timeout
			if(aEvent.timestamp - subscriber->second.lastActiveTimestamp > request_timeout_1_sec_ms)
			{
				subscriber->second.waitingForRequestAcknowledge = false;
				return std::nullopt;
			}

			// check request flag
			if(!subscriber->second.waitingForRequestAcknowledge)
				return std::nullopt;

			subscriber->second.lastActiveTimestamp = aEvent.timestamp;
			subscriber->second.waitingForRequestAcknowledge = false;

			// detache ids form imsi
			m_enodeb_id2imsi.erase(aEvent.enodeb_id.value());
			m_mme_id2imsi.erase(aEvent.mme_id.value());

			return {{
					.s1ap_type = S1apOut::S1apOutType::UnReg,
					.imsi = imsi,
					.cgi = aEvent.cgi.value()
				}};
		}
	}

	return std::nullopt;
}

} // namespace demo