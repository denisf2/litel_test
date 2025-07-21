#include "S1apDb.h"

namespace demo
{

auto S1apDb::handler(const Event& aEvent) -> std::optional<S1apOut>
{
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
			// TODO: return handlePaging();
		case ET::PathSwitchRequest:
			// TODO: return handlePathSwitchRequest();
		case ET::PathSwitchRequestAcknowledge:
			// TODO: return handlePathSwitchRequestAcknowledge();
		case ET::UEContextReleaseCommand:
			// TODO: return handleUEContextReleaseCommand();
		case ET::UEContextReleaseResponse:
			// TODO: return handleUEContextReleaseResponse();
		default:
			return std::nullopt;
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

} // namespace demo