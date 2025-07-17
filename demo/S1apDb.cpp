#include "demo.h"
#include "S1apDb.h"

namespace demo
{

auto S1apDb::handler(Event aEvent) -> std::optional<S1apOut>
{
	switch(aEventType.event_type)
	{
		case EventType::AttachRequest:
		{
		} 
	}
}

} // namespace demo