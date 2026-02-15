#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "input/input_events.h"

class IGuiWidget
{
	public:
		IGuiWidget() = default;
		virtual ~IGuiWidget() = default;
		virtual void Initialize() { bIsInitialized = true; }
		virtual void Stage(std::vector<InputEvent>& outEvents) { assert(bIsInitialized && "Widget not Initialized"); }
		[[nodiscard]] bool IsInitialized() const { return bIsInitialized; }

	protected:
		bool bIsInitialized = false;
};

#endif