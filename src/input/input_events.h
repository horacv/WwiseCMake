#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

struct QuitRequestedEvent {};
struct ToggleAudioInfoOverlayEvent {};
struct OpenPageEvent { std::string page_name = std::string(); };

using InputEvent = std::variant<
	QuitRequestedEvent,
	ToggleAudioInfoOverlayEvent,
	OpenPageEvent
>;

#endif