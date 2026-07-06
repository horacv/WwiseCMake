#ifndef PAGE_EXTERNAL_SOURCES_H
#define PAGE_EXTERNAL_SOURCES_H

#include "page.h"

#include "audio/audio_engine.h"

struct MediaInfo
{
    std::string mediaName;
    std::string ExtSourceCookieName;
    AkCodecID codecID;
    bool bIsStreamed {true};
};

struct EventAndMediaInfo
{
    std::string eventName;
    std::string uiLabel;
    std::vector<MediaInfo> mediaInfo;
};

using InMemoryAudioData = std::vector<std::byte>;

class PageExternalSources : public IPage
{
public:
    PageExternalSources();
    void Initialize() override;
    void Deinitialize() override;

protected:
    void Start() override;
    void RenderStage() override;

private:
    SDL_Surface* titleTextSurface;
    SDL_Texture* titleTextTexture;

    uint64_t audioObjectID;
    uint32_t currentAudioPlayingID;
    EventAndMediaInfo selectedEventAndMediaInfo;

    std::mutex audioInMemoryMutex;
    std::multimap<uint32_t, InMemoryAudioData> currentAudioInMemory;
    std::multimap<uint32_t, std::string> currentPlayingMedia;

    bool bReverbEnabled;

    void StageExternalSourceList();
    void PlayExternalSources(const EventAndMediaInfo& eventAndMediaInfo);
    void HandleClearUnusedResources(uint32_t audioInstanceID);

    static bool LoadFile(const std::filesystem::path& path, std::vector<std::byte>& outData);
    static void ExternalSourceEventCallback(AudioCallbackType type,
        AudioEventCallbackInfo* eventInfo, void* callbackInfo, void* cookie);
};

#endif