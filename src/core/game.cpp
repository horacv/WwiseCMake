#include "game.h"

#include "audio/audio_engine.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <format>
#include <iostream>

Game::Game()
	: mIsRunning(false)
	, mWindowWidth(1024)
	, mWindowHeight(768)
	, mWindow(nullptr)
	, mRenderer(nullptr)
	, titleTextSurface(nullptr)
	, titleTextTexture(nullptr)
	, mCurrentMusicBar(0)
	, mCurrentMusicBeat(0)
{
	std::cout << "Game Created" << std::endl;
}

void Game::Initialize()
{
	AudioEngine::Initialize(); // AUDIO ENGINE INIT

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	mWindow = SDL_CreateWindow("Wwise is Alive!",mWindowWidth,mWindowHeight, 0);
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

	AudioEngine::Terminate(); // AUDIO ENGINE TERM

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
	constexpr SDL_Color fontColor = { 255, 255, 255, 255 };
	TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", fontPointSize);
	titleTextSurface = TTF_RenderText_Solid(font, "Wwise is Alive!", 0, fontColor);
	titleTextTexture = SDL_CreateTextureFromSurface(mRenderer, titleTextSurface);

	AudioEngine::LoadSoundBank("Music.bnk"); // AUDIO ENGINE: LOAD MUSIC BANK
	AudioEngine::PlayAudioEvent("MusicTest", AK_INVALID_AUDIO_OBJECT_ID,
		AK_MusicSyncAll, GlobalAudioEventCallback, this); // AUDIO ENGINE: PLAY MUSIC EVENT

	TTF_CloseFont(font);

	std::cout << "Game Started" << std::endl;
}

void Game::Render() const
{
	SDL_SetRenderDrawColor(mRenderer, 21, 21, 21, 255);
	SDL_RenderClear(mRenderer);

	// Main Text

	auto textPositionX = static_cast<float>(mWindowWidth) * 0.5f - static_cast<float>(titleTextSurface->w) * 0.5f;
	auto textPositionY = static_cast<float>(mWindowHeight) * 0.5f - static_cast<float>(titleTextSurface->h) * 0.5f;
	auto textWidth = static_cast<float>(titleTextSurface->w);
	auto textHeight = static_cast<float>(titleTextSurface->h);
	SDL_FRect textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
	SDL_RenderTexture(mRenderer, titleTextTexture, nullptr, &textRectangle);

	// Music Info Text

	auto [bar, beat] = GetCurrentMusicBarAndBeat();
	const std::string musicBeatText = std::format("Music Bar: {} Beat: {}", bar, beat);

	constexpr float fontPointSize = 40;
	constexpr SDL_Color fontColor = { 255, 255, 255, 255 };
	TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", fontPointSize);
	SDL_Surface* musicInfoTextSurface = TTF_RenderText_Solid(font, musicBeatText.c_str(), 0, fontColor);
	SDL_Texture* musicInfoTextTexture = SDL_CreateTextureFromSurface(mRenderer, musicInfoTextSurface);
	textPositionX = static_cast<float>(mWindowWidth) * 0.01f;
	textPositionY = static_cast<float>(mWindowHeight) * 0.94f;
	textWidth = static_cast<float>(musicInfoTextSurface->w * 0.75);
	textHeight = static_cast<float>(musicInfoTextSurface->h * 0.75);
	textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
	SDL_RenderTexture(mRenderer, musicInfoTextTexture, nullptr, &textRectangle);

	SDL_RenderPresent(mRenderer);

	SDL_DestroyTexture(musicInfoTextTexture);
	SDL_DestroySurface(musicInfoTextSurface);
	TTF_CloseFont(font);
}

void Game::GlobalAudioEventCallback(const AudioCallbackType type, AudioEventCallbackInfo* eventInfo, void* callbackInfo, void* cookie)
{
	if (!callbackInfo) { return; }

	if (type == AK_MusicSyncBeat)
	{
		const auto game = static_cast<Game*>(cookie);
		const auto* musicInfo = static_cast<AkMusicSyncCallbackInfo*>(callbackInfo);
		if (game && musicInfo)
		{
			const auto currentPositionMs = musicInfo->segmentInfo.iCurrentPosition + musicInfo->segmentInfo.iRemainingLookAheadTime;
			const auto barDurationMs = musicInfo->segmentInfo.fBarDuration * 1000;
			const auto rawBarPosition = static_cast<float>(currentPositionMs) / barDurationMs + 1;
			const auto roundedBarPosition = std::round(rawBarPosition * 100.0) * 0.01;
			const auto currentBar = static_cast<int>(roundedBarPosition);
			auto [prevBar, prevBeat] = game->GetCurrentMusicBarAndBeat();
			const auto currentBeat = currentBar == prevBar ? ++prevBeat : 1;

			game->SetCurrentMusicBarAndBeat(currentBar, currentBeat);
		 }
	}

	// Add more callback types here
	//...

	//...
}

void Game::SetCurrentMusicBarAndBeat(const int bar, const int beat)
{
	std::lock_guard lock(mMusicDataMutex);
	mCurrentMusicBar = bar;
	mCurrentMusicBeat = beat;
}

std::pair<int, int> Game::GetCurrentMusicBarAndBeat() const
{
	std::lock_guard lock(mMusicDataMutex);
	return {mCurrentMusicBar, mCurrentMusicBeat};
}
