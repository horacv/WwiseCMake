#ifndef GAME_HPP
#define GAME_HPP

#include "audio/audio_engine.h"
#include <SDL3/SDL.h>
#include <mutex>

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

		void Update();
		void ProcessInput();
		void Start();
		void Render() const;

		/**
		 * Audio Event Callback.
		 * Refer to: https://www.audiokinetic.com/en/public-library/2024.1.6_8842/?source=SDK&id=soundengine_music_callbacks.html
		 * "The callbacks are done from the sound engine's main thread.
		 * This means that your application should gather all the information it needs from the notification and return immediately.
		 * If any processing needs to be done, it should be performed in a separate thread after the relevant information has been copied from the notification."
		 */
		static void GlobalAudioEventCallback(AudioCallbackType type, AudioCallbackInfo* info);

		void SetCurrentMusicBarAndBeat(int bar, int beat);
		std::pair<int, int> GetCurrentMusicBarAndBeat() const;
};
#endif
