#include "app.h"

#include "audio/audio_engine.h"
#include "gui/gui.h"
#include "media/media_framework.h"
#include "media/media_framework_data.h"
#include "pages/page_cover.h"
#include "pages/page_external_sources.h"

const MediaWindowSettings INIT_WINDOW_SETTINGS{"Wwise is Alive!", 1024, 768};
constexpr SDL_Color BACKGROUND_COLOR{21, 21, 21, 255};

namespace
{
	const std::unordered_map<std::string_view, std::function<std::unique_ptr<IPage>()>> pages = {
		{"Cover", [](){ return std::make_unique<PageCover>(); }},
		{"External Sources", [](){ return std::make_unique<PageExternalSources>(); }}
	};
}

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
	currentPage->Initialize();
	std::cout << "Application Started" << std::endl;
}

void Application::ProcessEvents()
{
	MediaFramework::PollEvents(inputEventsCurrent);
	for (auto& inputEvent : inputEventsCurrent)
	{
		if (std::holds_alternative<QuitRequestedEvent>(inputEvent)) { mIsRunning = false; }
		if (std::holds_alternative<OpenPageEvent>(inputEvent))
		{
			ChangePage(std::get<OpenPageEvent>(inputEvent).page_name);
		}
	}
	inputEventsCurrent.clear();
}

void Application::Update()
{
	AudioEngine::Update();
	HandlePagesPendingDestroy();
}

void Application::Render()
{
	MediaFramework::RenderClear(BACKGROUND_COLOR);
	GUI::RenderClear();

	MediaFramework::RenderStage();
	GUI::RenderStage(inputEventsCurrent);

	MediaFramework::RenderPresent();
}

void Application::ChangePage(const std::string_view& pageName)
{
	if (const auto it = pages.find(pageName); it != pages.end())
	{
		std::unique_ptr newPage = it->second();

		if (currentPage)
		{
			currentPage->Deinitialize();
			mPagesPendingDestroy.push_back(std::move(currentPage));
		}

		currentPage = std::move(newPage);
		if (currentPage)
		{
			currentPage->Initialize();
		}
	}
}
void Application::HandlePagesPendingDestroy()
{
	for (auto it = mPagesPendingDestroy.begin(); it != mPagesPendingDestroy.end();)
	{
		IPage* page = it->get();
		if (!page)
		{
			++it; continue;
		}
		if (page->CanDestroy() && !page->IsInitialized())
		{
			it = mPagesPendingDestroy.erase(it);
		}
		else
		{
			++it;
		}
	}
}
