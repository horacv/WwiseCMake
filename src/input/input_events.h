#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

struct QuitRequestedEvent {};

using InputEvent = std::variant<
	QuitRequestedEvent>;

#endif