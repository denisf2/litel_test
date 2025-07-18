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
			//return handleIdentityResponse();
		case EventType::AttachAccept:
			//return handleAttachAccept();
		case EventType::Paging:
			//return handlePaging();
		case EventType::PathSwitchRequest:
			//return handlePathSwitchRequest();
		case EventType::PathSwitchRequestAcknowledge:
			//return handlePathSwitchRequestAcknowledge();
		case EventType::UEContextReleaseCommand:
			//return handleUEContextReleaseCommand();
		case EventType::UEContextReleaseResponse:
			//return handleUEContextReleaseResponse();
		default:
			return std::nullopt;
	}
}

auto handleAttachRequest() -> void
{

}

} // namespace demo