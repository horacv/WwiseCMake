#ifndef MEDIA_FRAMEWORK_H
#define MEDIA_FRAMEWORK_H

#include "audio/audio_engine.h"
#include "input/input_events.h"
#include "media_framework_data.h"

class IPage;

using RendereablePageSet = std::set<std::weak_ptr<IPage>, std::owner_less<std::weak_ptr<IPage>>>;

class MediaFramework
{
public:
    static MediaFramework& Get();

    static bool Initialize(const MediaWindowSettings& settings);
    static void Start();
    static void Terminate();

    static void PollEvents(std::vector<InputEvent>& outEvents);
    static void RenderClear(const SDL_Color& backgroundColor);
    static void RenderStage();
    static void RenderPresent();

    static bool GetRenderer(SDL_Renderer*& outRenderer);
    static bool GetWindow(SDL_Window*& outWindow);
    static const MediaWindowSettings& GetCurrentWindowSettings();

    static void SubscribeToRenderStage(const std::weak_ptr<IPage>& rendereable);

    static bool IsInitialized();

private:
    static std::unique_ptr<MediaFramework> sInstance;

    bool bIsInitialized;

    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    MediaWindowSettings mCurrentWindowSettings;

    RendereablePageSet mRenderablePages;

    MediaFramework();


};
#endif