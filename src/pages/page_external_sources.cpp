// Refer to: https://www.audiokinetic.com/en/public-library/2024.1.12_9034/?source=SDK&id=integrating_external_sources.html

/*
 * This page shows how to play external media using external sources in Wwise.
 * It shows how to play media from memory or streamed from disk.
 *
 * By using the event callback on AK_EndOfEvent, it's possible to know the precise time when the memory buffer can be freed.
 *
 */

#include "page_external_sources.h"

#include "media/media_framework.h"

#define FONT_PATH "assets/fonts/arial.ttf"

namespace
{
    // UI
    constexpr float fontPointSizeTitle = 48;
    constexpr SDL_Color titleFontColor = { 255, 255, 255, 255 };
    constexpr auto window_size = ImVec2(880, 480);
    constexpr auto window_pivot = ImVec2(0.5f, 0.5f);
    constexpr int maxVisibleListEntries = 12;
    const std::string& title = "External Sources";

    // AUDIO
    const std::string& soundbankName = "ExternalSources.bnk";
    const std::string& audioObjectName = "External Sources Page Object";

    const std::string& externalSourceSingle = "ExternalSource_Single";
    const std::string& externalSourceMulti = "ExternalSource_Multi";

    const std::string& reverbOnEventName = "ExternalSources_ReverbOn";
    const std::string& reverbOffEventName = "ExternalSources_ReverbOff";

    const std::string& mediaName_1 = "1_FromMemory_WemOpus.wem";
    const std::string& mediaName_2 = "2_Streamed_Vorbis.wem";
    const std::string& mediaName_3 = "3_FromMemory_ADPCM.wem";
    const std::string& mediaName_4 = "4_Streamed_PCM.wem";
    const std::string& mediaName_5_1 = "5.1_Streamed_Sequence_WemOpus.wem";
    const std::string& mediaName_5_2 = "5.2_Streamed_Sequence_WemOpus.wem";
    const std::string& mediaName_5_3 = "5.3_Streamed_Sequence_WemOpus.wem";

    std::vector<EventAndMediaInfo> eventsAndMedia = {
        {externalSourceSingle, mediaName_1 + " - Single - Codec: Wem Opus",
            {{mediaName_1, externalSourceSingle, AKCODECID_AKOPUS_WEM, false}}},

        {externalSourceSingle, mediaName_2 + " - Single - Codec: Vorbis",
            {{mediaName_2, externalSourceSingle, AKCODECID_VORBIS, true}}},

        {externalSourceSingle, mediaName_3 + " - Single - Codec: ADPCM",
            {{mediaName_3, externalSourceSingle, AKCODECID_ADPCM, false}}},

        {externalSourceSingle, mediaName_4 + " - Single - Codec: PCM",
            {{mediaName_4, externalSourceSingle, AKCODECID_PCM, true}}},

        {externalSourceMulti, "5.[1-3]_Streamed_Sequence_WemOpus.wem - Sequence Container (Continuous) - Codec: Wem Opus",
            {
                {mediaName_5_1, externalSourceMulti + "_00", AKCODECID_AKOPUS_WEM, true},
                {mediaName_5_2, externalSourceMulti + "_01", AKCODECID_AKOPUS_WEM, true},
                {mediaName_5_3, externalSourceMulti + "_02", AKCODECID_AKOPUS_WEM, true}}},
    };

}

PageExternalSources::PageExternalSources()
    : titleTextSurface(nullptr)
    , titleTextTexture(nullptr)
    , audioObjectID(AK_INVALID_AUDIO_OBJECT_ID)
    , currentAudioPlayingID(AK_INVALID_UNIQUE_ID)
    , selectedEventAndMediaInfo(eventsAndMedia[0])
    , bReverbEnabled(false)
{}

void PageExternalSources::Initialize()
{
    IPage::Initialize();
    MediaFramework::SubscribeToRenderStage(weak_from_this());

    bCanDestroy.store(false, std::memory_order_release);
    Start();
}

void PageExternalSources::Deinitialize()
{
    IPage::Deinitialize();
    MediaFramework::UnsubscribeFromRenderStage(weak_from_this());

    if (audioObjectID == AK_INVALID_AUDIO_OBJECT_ID || currentAudioPlayingID == AK_INVALID_UNIQUE_ID)
    {
        bCanDestroy.store(true, std::memory_order_release);
    }

    AudioEngine::StopPlayingAudioInstance(currentAudioPlayingID);
    AudioEngine::CancelAllCallbacksForAudioObject(audioObjectID);
    AudioEngine::AudioObjectUnregister(audioObjectID);
    AudioEngine::SoundbankUnload(soundbankName);

    if (titleTextTexture) { SDL_DestroyTexture(titleTextTexture); }
    if (titleTextSurface) { SDL_DestroySurface(titleTextSurface); }
}

void PageExternalSources::Start()
{
    if (SDL_Renderer* renderer; MediaFramework::GetRenderer(renderer))
    {
        TTF_Font* font = TTF_OpenFont(FONT_PATH, fontPointSizeTitle);
        titleTextSurface = TTF_RenderText_Solid(font, title.c_str(), 0, titleFontColor);
        titleTextTexture = SDL_CreateTextureFromSurface(renderer, titleTextSurface);
        TTF_CloseFont(font);
    }

    if (AudioEngine::SoundbankLoad(soundbankName))
    {
        audioObjectID = AudioEngine::AudioObjectGetNewID();
        AudioEngine::AudioObjectRegister(audioObjectID, audioObjectName);
    }
}

void PageExternalSources::StageExternalSourceList()
{
    for (const auto& info : eventsAndMedia)
    {
        const bool bSelected = selectedEventAndMediaInfo.uiLabel == info.uiLabel;
        const ImGuiSelectableFlags_ flags =  bSelected ? ImGuiSelectableFlags_Highlight: ImGuiSelectableFlags_None;
        if (ImGui::Selectable(info.uiLabel.c_str(), bSelected, flags))
        {
            selectedEventAndMediaInfo = info;
        }
    }
}

void PageExternalSources::PlayExternalSources(const EventAndMediaInfo& eventAndMediaInfo)
{
    if (audioObjectID == AK_INVALID_AUDIO_OBJECT_ID) { return; }

    std::vector<std::filesystem::path> paths;
    std::vector<AudioExternalSourceInfo> media;
    std::vector<InMemoryAudioData> newInMemoryData;

#ifdef AK_OS_WCHAR // Windows only!
    std::vector<std::wstring> mediaPaths;
#else
    std::vector<std::string> mediaPaths;
#endif

    for (const auto& [mediaName, ExtSourceCookieName, codecID, bIsStreamed] : eventAndMediaInfo.mediaInfo)
    {
        AudioExternalSourceInfo newMedia;
        newMedia.idCodec = codecID;
        newMedia.iExternalSrcCookie = AudioEngine::GetAudioIDFromName(ExtSourceCookieName);

        if (bIsStreamed)
        {
            // To stream media, provide a path name relative to the folder provided to the IO manager
            // See mLowLevelIO.SetBasePath in audio_engine.cpp
            paths.emplace_back(AudioEngine::GetExternalSourcesSubFolder().data() + mediaName);

#ifdef AK_OS_WCHAR // Windows only!
            mediaPaths.push_back(paths.back().wstring());
#else
            mediaPaths.push_back(paths.back().string());
#endif
            newMedia.szFile = mediaPaths.back().data();
        }
        else
        {
            // To play from memory,provide a path name relative to the executable or a fully qualified path
            paths.emplace_back(AudioEngine::GetExternalSourcesBasePath().data() + mediaName);

            InMemoryAudioData newInMemoryDataEntry;
            if (!LoadFile(paths.back(), newInMemoryDataEntry))
            {
                std::cout << std::format("Failed to load file: {} in memory", paths.back().string())  << std::endl;
                return;
            }

            // Memory buffers need to persist during the lifetime of the media playback
            newInMemoryData.push_back(std::move(newInMemoryDataEntry));
            newMedia.pInMemory = newInMemoryData.back().data();
            newMedia.uiMemorySize = newInMemoryData.back().size();
        }

        media.push_back(newMedia);
    }

    if (!media.empty())
    {
        AudioEngine::StopPlayingAudioInstance(currentAudioPlayingID);
        currentAudioPlayingID = AudioEngine::PlayAudioEvent(eventAndMediaInfo.eventName, audioObjectID,
        AK_EndOfEvent, &ExternalSourceEventCallback, this, media);

        if (currentAudioPlayingID == AK_INVALID_PLAYING_ID) { return; }

        std::lock_guard lock(audioInMemoryMutex);
        for (auto& newEntry: newInMemoryData)
        {
            currentAudioInMemory.emplace(currentAudioPlayingID, std::move(newEntry));
        }

        for (auto& newEntry : eventAndMediaInfo.mediaInfo)
        {
            currentPlayingMedia.emplace(currentAudioPlayingID, newEntry.mediaName);
        }
    }
}

void PageExternalSources::HandleClearUnusedResources(const uint32_t audioInstanceID)
{
    std::lock_guard lock(audioInMemoryMutex);
    currentAudioInMemory.erase(audioInstanceID);
    currentPlayingMedia.erase(audioInstanceID);

    if (currentAudioPlayingID == audioInstanceID)
    {
        currentAudioPlayingID = AK_INVALID_UNIQUE_ID;
        bCanDestroy.store(currentAudioInMemory.empty(), std::memory_order_release);
    }
}

bool PageExternalSources::LoadFile(const std::filesystem::path& path, std::vector<std::byte>& outData)
{
    outData.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file) { return false; }

    outData.resize(std::filesystem::file_size(path));
    file.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(outData.size()));

    return !outData.empty();
}

void PageExternalSources::RenderStage()
{
    SDL_Renderer* renderer;
    MediaFramework::GetRenderer(renderer);
    const MediaWindowSettings& windowSettings = MediaFramework::GetCurrentWindowSettings();

    // Title Text

    const auto textPositionX = static_cast<float>(windowSettings.width) * 0.5f - static_cast<float>(titleTextSurface->w) * 0.5f;
    const auto textPositionY = static_cast<float>(windowSettings.height) * 0.12f - static_cast<float>(titleTextSurface->h) * 0.5f;
    const auto textWidth = static_cast<float>(titleTextSurface->w);
    const auto textHeight = static_cast<float>(titleTextSurface->h);
    const SDL_FRect textRectangle = {textPositionX, textPositionY, textWidth, textHeight};
    SDL_RenderTexture(renderer, titleTextTexture, nullptr, &textRectangle);

    // IMGUI Window

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 window_pos {
        (viewport->WorkPos.x + viewport->WorkSize.x) * 0.5f,
        (viewport->WorkPos.y + viewport->WorkSize.y) * 0.5f
    };

    constexpr ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pivot);
    ImGui::SetNextWindowSize(window_size);
    if (ImGui::Begin(title.c_str(), nullptr, window_flags))
    {
        if (ImGui::BeginListBox("##External Sources Inner Box",
            ImVec2(-FLT_MIN, maxVisibleListEntries * ImGui::GetTextLineHeightWithSpacing())))
        {
            StageExternalSourceList();
            ImGui::EndListBox();
        }
        ImGui::Spacing();

        ImGui::PushID("ExtSourceButtonPlayButton"); // ImColor::Green
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.8f, 0.8f)));
        ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.5f);
        if (ImGui::Button("PLAY"))
        {
            PlayExternalSources(selectedEventAndMediaInfo);
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);
        ImGui::PopID();

        ImGui::SameLine();

        ImGui::PushID("ExtSourceButtonStopButton"); // ImColor::Red
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.8f, 0.8f)));
        ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.5f);
        if (ImGui::Button("STOP"))
        {
            AudioEngine::StopPlayingAudioInstance(currentAudioPlayingID);
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);
        ImGui::PopID();

        ImGui::SameLine();

        if (ImGui::Checkbox("Enable Reverb", &bReverbEnabled))
        {
            if (audioObjectID != AK_INVALID_AUDIO_OBJECT_ID)
            {
                AudioEngine::PlayAudioEvent(bReverbEnabled ? reverbOnEventName : reverbOffEventName, audioObjectID);
            }
        }

        for (int i = 0; i < 2; ++i) { ImGui::Spacing(); }

        constexpr ImGuiWindowFlags info_window_flags =
                ImGuiChildFlags_Borders
              | ImGuiChildFlags_AutoResizeX
              | ImGuiChildFlags_AutoResizeY;

        if (ImGui::BeginChild("Info Overlay", ImVec2(0.0f, 0.0f), info_window_flags))
        {
            if (currentPlayingMedia.empty())
            {
                ImGui::Text("No Playback");
            }
            else
            {
                ImGui::Text("Now Playing: ");
                ImGui::Spacing();

                for (const auto& media : currentPlayingMedia | std::views::values)
                {
                    ImGui::Text("   - %s", media.c_str());
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void PageExternalSources::ExternalSourceEventCallback(AudioCallbackType type, AudioCallbackInfo* info)
{
    if (!info) { return; }
    const auto page = static_cast<PageExternalSources*>(info->pCookie);
    const auto eventCallbackInfo = static_cast<AkEventCallbackInfo*>(info);
    if (!(page && eventCallbackInfo)) { return; }

    if (type == AK_EndOfEvent)
    {
        page->HandleClearUnusedResources(eventCallbackInfo->playingID);
    }
}

#undef FONT_PATH