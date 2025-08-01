#include "S1apDb.h"

namespace demo
{

auto operator== (const S1apOut& aLeft, const S1apOut& aRight) -> bool
{
	return aLeft.s1ap_type == aRight.s1ap_type
			&& aLeft.imsi == aRight.imsi
			&& aLeft.cgi == aRight.cgi
		;
}

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
			return handleIdentityResponse(aEvent);
		case ET::AttachAccept:
			 return handleAttachAccept(aEvent);
		case ET::Paging:
			return handlePaging(aEvent);
		case ET::PathSwitchRequest:
			return handlePathSwitchRequest(aEvent);
		case ET::PathSwitchRequestAcknowledge:
			return handlePathSwitchRequestAcknowledge(aEvent);
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

	if(m_timeStampQueue.empty())
		return;

	// taking the oldest Imsi
	const auto& [_, topImsi] = m_timeStampQueue.top();

	// check if it is still here
	const auto it = m_subscribers.find(topImsi);

	//invalidates ref
	m_timeStampQueue.pop();

	if(m_subscribers.end() == it)
		return;

	const auto& [imsi, subscriber] = *it;

	// check how old it`s session is
	if(aCurrentTime - subscriber.lastActiveTimestamp <= session_timeout_24_hours_ms)
	{
		// is still worthy then update time and push it back
		m_timeStampQueue.emplace(subscriber.lastActiveTimestamp, imsi);
		return;
	}

	// time to pass away
	m_m_tmsi2imsi.erase(subscriber.m_tmsi);
	m_enodeb_id2imsi.erase(subscriber.enodeb_id);
	m_mme_id2imsi.erase(subscriber.mme_id);

	m_subscribers.erase(imsi);
}

auto S1apDb::handleAttachRequest(const Event& aEvent)->std::optional<S1apOut>
{
	// AttachRequest{enodeb_id_1, imsi_1, cgi_1}
	// less 24 hours
	// AttachRequest {enodeb_id_3, m_tmsi_1, cgi_5}
	// more 24 hours
	// AttachRequest {enodeb_id_5, imsi_1, cgi_8}

	return aEvent.imsi ? handleAttachRequest_imsi(aEvent)
				: aEvent.m_tmsi ? handleAttachRequest_m_tmsi(aEvent)
					: std::nullopt;
}

auto S1apDb::handleAttachRequest_imsi(const Event& aEvent) -> std::optional<S1apOut>
{
	//                 new        new|old  new
	// AttachRequest{enodeb_id_1, imsi_1, cgi_1}

	// ismi is unique but in case double messages do not produce new it
	const auto imsi = aEvent.imsi.value();
	// TODO: double run unordered_map lookup. insert, emplace, insert_or_assign overwrites full structure
	// TODO: unordered_map.find makes one pass
	const auto newSubscriber = not m_subscribers.contains(imsi);
	auto& subscriber = m_subscribers[imsi];

	subscriber.lastActiveTimestamp = aEvent.timestamp;
	subscriber.enodeb_id = aEvent.enodeb_id.value();
	subscriber.cgi = aEvent.cgi.value();

	subscriber.waitingForAttachAccept = true;
	subscriber.waitingForIdentityResponse = true;

	m_enodeb_id2imsi[aEvent.enodeb_id.value()] = imsi;

	// suscriber`s session timeout managment
	m_timeStampQueue.emplace(aEvent.timestamp, imsi);

	// TODO: i`m not sure here about s1ap_type
	if(newSubscriber)
	{
		return {{
				.s1ap_type = S1apOut::S1apOutType::Reg,
				.imsi = imsi,
				.cgi = aEvent.cgi.value()
			}};
	}

	return std::nullopt;
}

auto S1apDb::handleAttachRequest_m_tmsi(const Event& aEvent) -> std::optional<S1apOut>
{
	//                  new        new|old    new
	// AttachRequest {enodeb_id_3, m_tmsi_1, cgi_5}
	// find old imsi and update
	// m_tmsi -> imsi -> new / update it
	if(const auto imsiIter = m_m_tmsi2imsi.find(aEvent.m_tmsi.value()); m_m_tmsi2imsi.cend() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		// TODO: double run unordered_map lookup. insert, emplace, insert_or_assign overwrites full structure
		// TODO: unordered_map.find makes one pass
		const auto newSubscriber = not m_subscribers.contains(imsi);
		auto& subscriber = m_subscribers[imsi];

		subscriber.lastActiveTimestamp = aEvent.timestamp;
		subscriber.enodeb_id = aEvent.enodeb_id.value();
		subscriber.m_tmsi = aEvent.m_tmsi.value();
		subscriber.cgi = aEvent.cgi.value();

		subscriber.waitingForAttachAccept = true;
		subscriber.waitingForIdentityResponse = true;

		m_enodeb_id2imsi[aEvent.enodeb_id.value()] = imsi;

		// suscriber`s session timeout managment
		m_timeStampQueue.emplace(aEvent.timestamp, imsi);

		// TODO: i`m not sure here about old/new it and s1ap_type
		// suppose always restore old it
		if(newSubscriber)
		{
			// TODO: this never sends
			return {{
					.s1ap_type = S1apOut::S1apOutType::Reg,
					.imsi = imsi,
					.cgi = aEvent.cgi.value()
				}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handleIdentityResponse(const Event& aEvent) -> std::optional<S1apOut>
{
	//                                         old     new
	//IdentityResponse{enodeb_id_1, mme_id_1, imsi_1, cgi_2}

	if(auto it = m_subscribers.find(aEvent.imsi.value()); m_subscribers.end() != it)
	{
		auto& [imsi, subscriber] = *it;
		if(!subscriber.waitingForIdentityResponse)
		{
			subscriber.lastActiveTimestamp = aEvent.timestamp;
			return std::nullopt;
		}

		subscriber.waitingForIdentityResponse = false;

		// check request timeout condition
		if(aEvent.timestamp - subscriber.lastActiveTimestamp > request_timeout_1_sec_ms)
		{
			subscriber.lastActiveTimestamp = aEvent.timestamp;
			return std::nullopt;
		}

		subscriber.lastActiveTimestamp = aEvent.timestamp;
		subscriber.cgi = aEvent.cgi.value();

		// TODO: return struct on change cgi or always?
		return {{
				.s1ap_type = S1apOut::S1apOutType::Cgi,
				.imsi = imsi,
				.cgi = aEvent.cgi.value()
			}};
	}

	return std::nullopt;
}

auto S1apDb::handleAttachAccept(const Event& aEvent) -> std::optional<S1apOut>
{
	//                         new      new
	//AttachAccept{enodeb_id, mme_id, m_tmsi}

	if(const auto imsiIter = m_enodeb_id2imsi.find(aEvent.enodeb_id.value()); m_enodeb_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			// check request flag
			if(!subscriber.waitingForAttachAccept)
				return std::nullopt;

			subscriber.waitingForAttachAccept = false;

			// check timeout
			if(aEvent.timestamp - subscriber.lastActiveTimestamp > request_timeout_1_sec_ms)
				return std::nullopt;

			// TODO: ??? shoud mme update it last active time?
			//it.lastActiveTimestamp = aEvent.timestamp;

			// update it
			subscriber.mme_id = aEvent.mme_id.value();
			subscriber.m_tmsi = aEvent.m_tmsi.value();

			// update indexes
			m_m_tmsi2imsi[aEvent.m_tmsi.value()] = imsi;
			m_mme_id2imsi[aEvent.mme_id.value()] = imsi;

			return {{
					.s1ap_type = S1apOut::S1apOutType::Reg,
					.imsi = _imsi_,
					.cgi = subscriber.cgi
				}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handlePaging(const Event& aEvent) -> std::optional<S1apOut>
{
	//                new
	// Paging{m_tmsi, cgi}

	if(const auto imsiIter = m_m_tmsi2imsi.find(aEvent.m_tmsi.value()); m_m_tmsi2imsi.cend() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			subscriber.lastActiveTimestamp = aEvent.timestamp;
			subscriber.cgi = aEvent.cgi.value();

			// TODO: return struct on change cgi or always?
			return {{
						.s1ap_type = S1apOut::S1apOutType::Cgi,
						.imsi = _imsi_,
						.cgi = aEvent.cgi.value(),
					}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handlePathSwitchRequest(const Event& aEvent) -> std::optional<S1apOut>
{
	//                     new          old      new
	// PathSwitchRequest{enodeb_id_4, mme_id_3, cgi_6}

	if(const auto imsiIter = m_mme_id2imsi.find(aEvent.mme_id.value()); m_mme_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			subscriber.lastActiveTimestamp = aEvent.timestamp;
			subscriber.waitingForRequestAcknowledge = true;

			// TODO: ??? do i need update cgi here without timeout check?
			subscriber.cgi = aEvent.cgi.value();

			// TODO: return struct on change cgi or always?
			return {{
						.s1ap_type = S1apOut::S1apOutType::Cgi,
						.imsi = _imsi_,
						.cgi = aEvent.cgi.value(),
					}};
		}
	}

	return std::nullopt;
}

auto S1apDb::handlePathSwitchRequestAcknowledge(const Event& aEvent) -> std::optional<S1apOut>
{
	//                                  new         old
	// PathSwitchRequestAcknowledge {enodeb_id_4, mme_id_4}

	if(const auto imsiIter = m_mme_id2imsi.find(aEvent.mme_id.value()); m_mme_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			// check request flag
			if(!subscriber.waitingForRequestAcknowledge)
				return std::nullopt;

			subscriber.waitingForRequestAcknowledge = false;

			// timeout check
			if(aEvent.timestamp - subscriber.lastActiveTimestamp > request_timeout_1_sec_ms)
				return std::nullopt;

			// TODO: ??? shoud mme update it last active time?
			//subscriber.lastActiveTimestamp = aEvent.timestamp;

			m_enodeb_id2imsi.erase(subscriber.enodeb_id);
			m_enodeb_id2imsi[aEvent.enodeb_id.value()] = imsi;

			subscriber.enodeb_id = aEvent.enodeb_id.value();
		}
	}

	return std::nullopt;
}

auto S1apDb::handleUEContextReleaseCommand(const Event& aEvent) -> std::optional<S1apOut>
{
	//                                                  new
	// UEContextReleaseCommand {enodeb_id_4, mme_id_4, cgi_7}
	// TODO: what should i prefer enodeb_id or mme_id?
	if(const auto imsiIter = m_enodeb_id2imsi.find(aEvent.enodeb_id.value()); m_enodeb_id2imsi.end() != imsiIter)
	{
		const auto& imsi = imsiIter->second;
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			subscriber.lastActiveTimestamp = aEvent.timestamp;
			// TODO: ??? do i need update cgi here without timeout check?
			subscriber.cgi = aEvent.cgi.value();
			subscriber.waitingForReleaseResponse = true;

			// TODO: return struct on change cgi or always?
			return {{
					.s1ap_type = S1apOut::S1apOutType::Cgi,
					.imsi = _imsi_,
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
		if(auto it = m_subscribers.find(imsi); m_subscribers.end() != it)
		{
			auto& [_imsi_, subscriber] = *it;
			// check request flag
			if(!subscriber.waitingForReleaseResponse)
				return std::nullopt;

			subscriber.waitingForReleaseResponse = false;

			// check timeout
			if(aEvent.timestamp - subscriber.lastActiveTimestamp > request_timeout_1_sec_ms)
				return std::nullopt;

			// TODO: ??? shoud mme update it last active time?
			//subscriber.lastActiveTimestamp = aEvent.timestamp;

			// detache ids form imsi
			m_enodeb_id2imsi.erase(aEvent.enodeb_id.value());
			m_mme_id2imsi.erase(aEvent.mme_id.value());

			return {{
					.s1ap_type = S1apOut::S1apOutType::UnReg,
					.imsi = _imsi_,
					.cgi = subscriber.cgi
				}};
		}
	}

	return std::nullopt;
}

} // namespace demo