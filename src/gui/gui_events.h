#ifndef GUI_EVENTS_H
#define GUI_EVENTS_H

struct QuitRequestedEvent {};

using GUIEvent = std::variant<
	QuitRequestedEvent>;

#endif