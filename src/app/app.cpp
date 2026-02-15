#include "app.h"

#include "audio/audio_engine.h"
#include "gui/gui.h"
#include "media/media_framework.h"
#include "media/media_framework_data.h"
#include "pages/page_cover.h"
#include <iostream>

const MediaWindowSettings INIT_WINDOW_SETTINGS{"Wwise is Alive!", 1024, 768};
constexpr SDL_Color BACKGROUND_COLOR{21, 21, 21, 255};

Application::Application()
: mIsRunning(false)
, currentPage(std::make_unique<PageCover>())
{
	std::cout << "Application Created" << std::endl;
}

void Application::Initialize()
{
	if (!(AudioEngine::Initialize() && MediaFramework::Initialize(INIT_WINDOW_SETTINGS)))
	{
		assert(!"Failed to initialize application");
	}

	GUI::Initialize();
	mIsRunning = true;
	std::cout << "Application Initialized" << std::endl;
}

void Application::Run()
{
	Start();
	while (IsRunning())
	{
		ProcessEvents();
		Update();
		Render();
	}
}

void Application::Terminate()
{
	GUI::Terminate();
	MediaFramework::Terminate();
	AudioEngine::Terminate();

	std::cout << "Application terminated" << std::endl;
}

bool Application::IsRunning() const
{
	return mIsRunning;
}

void Application::Start() const
{
	MediaFramework::Start();
	InitializeCurrentPage();
	std::cout << "Application Started" << std::endl;
}

void Application::InitializeCurrentPage() const
{
	currentPage->Initialize();
	currentPage->Start();
}

void Application::ProcessEvents()
{
	MediaFramework::PollEvents(inputEventsCurrent);
	for (auto& guiEvent : inputEventsCurrent)
	{
		if (std::holds_alternative<QuitRequestedEvent>(guiEvent)) { mIsRunning = false; }
	}
	inputEventsCurrent.clear();
}

void Application::Update()
{
	AudioEngine::Update();
}

void Application::Render()
{
	MediaFramework::RenderClear(BACKGROUND_COLOR);
	GUI::RenderClear();

	MediaFramework::RenderStage();
	GUI::RenderStage(inputEventsCurrent);

	MediaFramework::RenderPresent();
}
