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

	static void GlobalAudioEventCallback(AudioCallbackType type, AudioCallbackInfo* info);

	void SetCurrentMusicBarAndBeat(int bar, int beat);
	std::pair<int, int> GetCurrentMusicBarAndBeat() const;
};
#endif
