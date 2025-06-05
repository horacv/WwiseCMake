#ifndef GAME_HPP
#define GAME_HPP

#include <SDL3/SDL.h>

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
		SDL_Surface* textSurface;
		SDL_Texture* textTexture;

		void Update();
		void ProcessInput();
		void Start();
		void Render() const;
};
#endif
