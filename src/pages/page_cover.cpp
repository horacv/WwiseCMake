#include "page_cover.h"

#include "media/media_framework.h"

#define FONT_PATH "assets/fonts/arial.ttf"

namespace
{
    // UI
    constexpr float fontPointSizeTitle = 96;
    constexpr float fontPointSizeBeatCounter = 40;
    constexpr SDL_Color fontColor = { 255, 255, 255, 255 };
    const std::string& title = "Wwise is Alive!";

    // AUDIO
    const std::string& soundbankName = "Music.bnk";
    const std::string& audioObjectName = "Cover Page Music Object";
    const std::string& audioEventName = "MusicTest";

}


PageCover::PageCover()
    : titleTextSurface(nullptr)
    , titleTextTexture(nullptr)
    , mCurrentMusicBar(0)
    , mCurrentMusicBeat(0)
    , musicInstanceID(0)
    , audioObjectID(0)
{}

void PageCover::Initialize()
{
    IPage::Initialize();
    MediaFramework::SubscribeToRenderStage(weak_from_this());

    Start();
}

void PageCover::Deinitialize()
{
    IPage::Deinitialize();
    MediaFramework::UnsubscribeFromRenderStage(weak_from_this());

    if (audioObjectID != AK_INVALID_AUDIO_OBJECT_ID)
    {
        AudioEngine::CancelAllCallbacksForAudioObject(audioObjectID);
        AudioEngine::StopPlayingAudioInstance(musicInstanceID);
        AudioEngine::AudioObjectUnregister(audioObjectID);
        AudioEngine::SoundbankUnload(soundbankName);
    }

    bCanDestroy.store(true, std::memory_order_release);

    if (titleTextTexture) { SDL_DestroyTexture(titleTextTexture); }
    if (titleTextSurface) { SDL_DestroySurface(titleTextSurface); }
}

void PageCover::Start()
{
    if (SDL_Renderer* renderer; MediaFramework::GetRenderer(renderer))
    {
        TTF_Font* font = TTF_OpenFont(FONT_PATH, fontPointSizeTitle);
        titleTextSurface = TTF_RenderText_Solid(font, title.c_str(), 0, fontColor);
        titleTextTexture = SDL_CreateTextureFromSurface(renderer, titleTextSurface);
        TTF_CloseFont(font);
    }

    if (AudioEngine::SoundbankLoad(soundbankName))
    {
        audioObjectID = AudioEngine::AudioObjectGetNewID();
        AudioEngine::AudioObjectRegister(audioObjectID, audioObjectName);
        musicInstanceID = AudioEngine::PlayAudioEvent(audioEventName, audioObjectID,
            AK_MusicSyncAll, MusicEventCallback, this);
    }
}

void PageCover::RenderStage()
{
    SDL_Renderer* renderer;
    MediaFramework::GetRenderer(renderer);
    const MediaWindowSettings& windowSettings = MediaFramework::GetCurrentWindowSettings();

    // Main Text

    auto textPositionX = static_cast<float>(windowSettings.width) * 0.5f - static_cast<float>(titleTextSurface->w) * 0.5f;
    auto textPositionY = static_cast<float>(windowSettings.height) * 0.5f - static_cast<float>(titleTextSurface->h) * 0.5f;
    auto textWidth = static_cast<float>(titleTextSurface->w);
    auto textHeight = static_cast<float>(titleTextSurface->h);
    SDL_FRect textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
    SDL_RenderTexture(renderer, titleTextTexture, nullptr, &textRectangle);

    // Music Info Text

    auto [bar, beat] = GetCurrentMusicBarAndBeat();
    const std::string musicBeatText = std::format("Music Bar: {} Beat: {}", bar, beat);

    TTF_Font* font = TTF_OpenFont(FONT_PATH, fontPointSizeBeatCounter);
    SDL_Surface* musicInfoTextSurface = TTF_RenderText_Solid(font, musicBeatText.c_str(), 0, fontColor);
    SDL_Texture* musicInfoTextTexture = SDL_CreateTextureFromSurface(renderer, musicInfoTextSurface);
    textPositionX = static_cast<float>(windowSettings.width) * 0.01f;
    textPositionY = static_cast<float>(windowSettings.height) * 0.94f;
    textWidth = static_cast<float>(musicInfoTextSurface->w * 0.75);
    textHeight = static_cast<float>(musicInfoTextSurface->h * 0.75);
    textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
    SDL_RenderTexture(renderer, musicInfoTextTexture, nullptr, &textRectangle);

    // Music Info Text: Render Cleanup

    SDL_DestroyTexture(musicInfoTextTexture);
    SDL_DestroySurface(musicInfoTextSurface);
    TTF_CloseFont(font);

}

void PageCover::MusicEventCallback(const AudioCallbackType type, AudioEventCallbackInfo* eventInfo, void* callbackInfo, void* cookie)
{
    if (!callbackInfo) { return; }

    if (type == AK_MusicSyncBeat)
    {
        const auto page = static_cast<PageCover*>(cookie);
        const auto* musicInfo = static_cast<AkMusicSyncCallbackInfo*>(callbackInfo);
        if (page && musicInfo)
        {
            const auto currentPositionMs = musicInfo->segmentInfo.iCurrentPosition + musicInfo->segmentInfo.iRemainingLookAheadTime;
            const auto barDurationMs = musicInfo->segmentInfo.fBarDuration * 1000;
            const auto rawBarPosition = static_cast<float>(currentPositionMs) / barDurationMs + 1;
            const auto roundedBarPosition = std::round(rawBarPosition * 100.0) * 0.01;
            const auto currentBar = static_cast<int>(roundedBarPosition);
            auto [prevBar, prevBeat] = page->GetCurrentMusicBarAndBeat();
            const auto currentBeat = currentBar == prevBar ? ++prevBeat : 1;

            page->SetCurrentMusicBarAndBeat(currentBar, currentBeat);
        }
    }
}

void PageCover::SetCurrentMusicBarAndBeat(const int bar, const int beat)
{
    std::lock_guard lock(mMusicDataMutex);
    mCurrentMusicBar = bar;
    mCurrentMusicBeat = beat;
}

std::pair<int, int> PageCover::GetCurrentMusicBarAndBeat() const
{
    std::lock_guard lock(mMusicDataMutex);
    return {mCurrentMusicBar, mCurrentMusicBeat};
}

#undef FONT_PATH