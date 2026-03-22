#ifndef PAGE_EXTERNAL_SOURCES_H
#define PAGE_EXTERNAL_SOURCES_H

#include "page.h"

#include "audio/audio_engine.h"

struct ExternalSourceInfo
{
    std::string name;
    std::string description;
    AkCodecID codecID;
    bool bIsStreamed {true};
};

class PageExternalSources : public IPage
{
public:
    PageExternalSources();
    void Initialize() override;
    void Deinitialize() override;
    void RenderStage() override;

private:
    SDL_Surface* titleTextSurface;
    SDL_Texture* titleTextTexture;

    uint64_t audioObjectID;
    ExternalSourceInfo selectedExternalSourceData;

    std::vector<std::byte> currentAudioInMemory;

    void Start() override;

    void StageExternalSourceList();
    void PlayExternalSource(const ExternalSourceInfo& info);
    void StopCurrentPlayback();

    bool LoadFile(const std::filesystem::path& path, std::vector<std::byte>& outData);
};


#endif