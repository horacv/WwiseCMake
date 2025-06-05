#include "game.h"

#include "audio/audio_engine.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>
#include <iostream>

Game::Game()
	: mIsRunning(false)
	, mWindowWidth(1024)
	, mWindowHeight(768)
	, mWindow(nullptr)
	, mRenderer(nullptr)
	, textSurface(nullptr)
	, textTexture(nullptr)
{
	std::cout << "Game Created" << std::endl;
}

void Game::Initialize()
{
	AudioEngine::Get().Initialize(); // AUDIO ENGINE INIT

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	mWindow = SDL_CreateWindow("Wwise is Alive!",mWindowWidth,mWindowHeight,SDL_WINDOW_ALWAYS_ON_TOP);
	mRenderer = SDL_CreateRenderer(mWindow, nullptr);
	mIsRunning = mWindow && mRenderer;

	std::cout << "Game Initialized" << std::endl;
}

void Game::Run()
{
	Start();
	while (IsRunning())
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Terminate() const
{
	TTF_Quit();
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();

	AudioEngine::Get().Terminate(); // AUDIO ENGINE TERM

	std::cout << "Game destroyed" << std::endl;
}

bool Game::IsRunning() const
{
	return mIsRunning;
}

void Game::Update()
{
	AudioEngine::RenderAudio(); // AUDIO ENGINE UPDATE - RENDER
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				mIsRunning = false;
				break;
			case SDL_EVENT_KEY_DOWN:
				if (event.key.key == SDLK_ESCAPE)
				{
					mIsRunning = false;
				}
				break;
			default:
				break;
		}
	}
}

void Game::Start()
{
	constexpr float fontPointSize = 96;
	TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", fontPointSize);
	constexpr SDL_Color fontColor = { 255, 255, 255, 255 };
	textSurface = TTF_RenderText_Solid(font, "Wwise is Alive!", 0, fontColor);
	textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);

	AudioEngine::LoadSoundBank("Music.bnk"); // AUDIO ENGINE: LOAD MUSIC BANK
	AudioEngine::PlayAudioEvent("MusicTest"); // AUDIO ENGINE: PLAY MUSIC EVENT

	std::cout << "Game Started" << std::endl;
}

void Game::Render() const
{
	SDL_SetRenderDrawColor(mRenderer, 21, 21, 21, 255);
	SDL_RenderClear(mRenderer);

	const auto textPositionX = static_cast<float>(mWindowWidth) * 0.5f - static_cast<float>(textSurface->w) * 0.5f;
	const auto textPositionY = static_cast<float>(mWindowHeight) * 0.5f - static_cast<float>(textSurface->h) * 0.5f;
	const auto textWidth = static_cast<float>(textSurface->w);
	const auto textHeight = static_cast<float>(textSurface->h);
	const SDL_FRect textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
	SDL_RenderTexture(mRenderer, textTexture, nullptr, &textRectangle);

	SDL_RenderPresent(mRenderer);
}
