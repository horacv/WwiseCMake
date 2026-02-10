#ifndef GAME_HPP
#define GAME_HPP

#include "audio/audio_engine.h"
#include "gui/gui_events.h"

class Game
{
	public:
		Game();

		void Initialize();
		void Run();
		void Terminate() const;
		[[nodiscard]] bool IsRunning() const;

	private:
		bool mIsRunning;

		int mWindowWidth;
		int mWindowHeight;
		SDL_Window* mWindow;
		SDL_Renderer* mRenderer;

		SDL_Surface* titleTextSurface;
		SDL_Texture* titleTextTexture;

		mutable std::mutex mMusicDataMutex;
		int mCurrentMusicBar;
		int mCurrentMusicBeat;

		std::vector<GUIEvent> guiEvents;

		void Start();
		void ProcessEvents();
		void ConsumeGUIEvents();
		void ConsumeInputEvents();
		void Update();
		void Render();

		/**
		 * Audio Event Callback.
		 * Refer to: https://www.audiokinetic.com/en/public-library/2025.1.2_8976/?source=SDK&id=soundengine_music_callbacks.html
		 * "The callbacks are done from the sound engine's main thread.
		 * This means that your application should gather all the information it needs from the notification and return immediately.
		 * If any processing needs to be done, it should be performed in a separate thread after the relevant information has been copied from the notification."
		 */
		static void GlobalAudioEventCallback(AudioCallbackType type,
			AudioEventCallbackInfo* eventInfo, void* callbackInfo, void* cookie);

		void SetCurrentMusicBarAndBeat(int bar, int beat);
		std::pair<int, int> GetCurrentMusicBarAndBeat() const;
};
#endif
