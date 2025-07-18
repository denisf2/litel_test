#include "demo.h"
#include "S1apDb.h"

namespace demo
{

auto handleAttachRequest() -> void;

auto S1apDb::handler(const Event& aEvent) -> std::optional<S1apOut>
{
	switch(aEventType.event_type)
	{
		case EventType::AttachRequest:
			return handleAttachRequest();
		case EventType::IdentityResponse:
			// TODO: return handleIdentityResponse();
		case EventType::AttachAccept:
			// TODO: return handleAttachAccept();
		case EventType::Paging:
			// TODO: return handlePaging();
		case EventType::PathSwitchRequest:
			// TODO: return handlePathSwitchRequest();
		case EventType::PathSwitchRequestAcknowledge:
			// TODO: return handlePathSwitchRequestAcknowledge();
		case EventType::UEContextReleaseCommand:
			// TODO: return handleUEContextReleaseCommand();
		case EventType::UEContextReleaseResponse:
			// TODO: return handleUEContextReleaseResponse();
		default:
			return std::nullopt;
	}
}

auto handleAttachRequest() -> void
{
	// TODO: create new subscriber
}

} // namespace demo