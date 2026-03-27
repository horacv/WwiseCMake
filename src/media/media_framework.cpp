#include "media_framework.h"

#include "gui/gui.h"
#include "pages/page.h"

std::unique_ptr<MediaFramework> MediaFramework::sInstance = nullptr;

MediaFramework::MediaFramework()
: bIsInitialized(false)
, mWindow(nullptr)
, mRenderer(nullptr)
{}

MediaFramework& MediaFramework::Get()
{
    if (!sInstance)
    {
        sInstance = std::unique_ptr<MediaFramework>(new MediaFramework());
    }
    return *sInstance;
}

bool MediaFramework::Initialize(const MediaWindowSettings& settings)
{
    MediaFramework& instance = Get();

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    instance.mWindow = SDL_CreateWindow(settings.title.c_str(), settings.width, settings.height, 0);
    instance.mRenderer = SDL_CreateRenderer(instance.mWindow, nullptr);
    instance.mCurrentWindowSettings = settings;
    instance.bIsInitialized = instance.mWindow && instance.mRenderer;

    return instance.bIsInitialized;
}

void MediaFramework::Start()
{
}

void MediaFramework::Terminate()
{
    const MediaFramework& instance = Get();
    assert(instance.bIsInitialized && "Trying to terminate uninitialized MediaFramework");

    TTF_Quit();
    SDL_DestroyRenderer(instance.mRenderer);
    SDL_DestroyWindow(instance.mWindow);
    SDL_Quit();

}

void MediaFramework::PollEvents(std::vector<InputEvent>& outEvents)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //TODO: Find a way to decouple the MediaFramework with the GUI class in the future. For now this is the simplest solution
        GUI::ProcessEvents(&event);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            outEvents.emplace_back(QuitRequestedEvent());
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) { outEvents.emplace_back(QuitRequestedEvent()); }
            break;
        default:
            break;
        }
    }
}

void MediaFramework::RenderClear(const SDL_Color& backgroundColor)
{
    const MediaFramework& instance = Get();

    SDL_SetRenderDrawColor(instance.mRenderer, backgroundColor.r,
        backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(instance.mRenderer);
}

void MediaFramework::RenderStage()
{
    MediaFramework& instance = Get();
    for (auto it = instance.mRenderablePages.begin(); it != instance.mRenderablePages.end();)
    {
        if (auto ptr = it->lock(); ptr == nullptr)
        {
            it = instance.mRenderablePages.erase(it);
        }
        else
        {
            ptr->RenderStage();
            ++it;
        }
    }
}

void MediaFramework::RenderPresent()
{
    const MediaFramework& instance = Get();
    SDL_RenderPresent(instance.mRenderer);
}

bool MediaFramework::GetRenderer(SDL_Renderer*& outRenderer)
{
    const MediaFramework& instance = Get();
    outRenderer = instance.mRenderer;
    return outRenderer;
}

bool MediaFramework::GetWindow(SDL_Window*& outWindow)
{
    const MediaFramework& instance = Get();
    outWindow = instance.mWindow;
    return outWindow;
}

const MediaWindowSettings& MediaFramework::GetCurrentWindowSettings()
{
    const MediaFramework& instance = Get();
    return instance.mCurrentWindowSettings;
}

void MediaFramework::SubscribeToRenderStage(const std::weak_ptr<IPage>& rendereable)
{
    MediaFramework& instance = Get();
    instance.mRenderablePages.insert(rendereable);
}

void MediaFramework::UnsubscribeFromRenderStage(const std::weak_ptr<IPage>& rendereable)
{
    MediaFramework& instance = Get();
    if (const auto page = instance.mRenderablePages.find(rendereable); page != instance.mRenderablePages.end())
    {
        instance.mRenderablePages.erase(page);
    }
}

bool MediaFramework::IsInitialized()
{
    const MediaFramework& instance = Get();
    return instance.bIsInitialized;
}

