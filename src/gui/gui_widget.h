#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "gui_events.h"

class IGuiWidget
{
	public:
		IGuiWidget() = default;
		virtual ~IGuiWidget() = default;
		virtual void Initialize() { bIsInitialized = true; }
		virtual void Stage(std::vector<GUIEvent>& outEvents) { assert(bIsInitialized && "Widget not Initialized"); }
		[[nodiscard]] bool IsInitialized() const { return bIsInitialized; }

	protected:
		bool bIsInitialized = false;
};

#endif