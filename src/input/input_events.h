#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

struct QuitRequestedEvent {};
struct ToggleAudioInfoOverlayEvent {};

using InputEvent = std::variant<
	QuitRequestedEvent,
	ToggleAudioInfoOverlayEvent>;

#endif