#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "input/input_events.h"

class IWidget
{
	public:
		IWidget() = default;
		virtual ~IWidget() = default;
		virtual void Initialize() { bIsInitialized = true; }
		virtual void Stage(std::vector<InputEvent>& outEvents) { assert(bIsInitialized && "Widget not Initialized"); }
		[[nodiscard]] bool IsInitialized() const { return bIsInitialized; }

	protected:
		bool bIsInitialized = false;
};

#endif