#include "page_external_sources.h"

#include "media/media_framework.h"

#define FONT_PATH "assets/fonts/arial.ttf"

namespace
{
    constexpr float fontPointSizeTitle = 96;
    constexpr float fontPointSizeBeatCounter = 40;
    constexpr SDL_Color fontColor = { 255, 255, 255, 255 };

    const std::string& externalSourceSingle = "ExternalSource_Single";
    const std::string& externalSourceMulti = "ExternalSource_Multi";

    const std::string& mediaName_1 = "1_FromMemory_WemOpus.wem";
    const std::string& mediaName_2 = "2_Streamed_Vorbis.wem";
    const std::string& mediaName_3 = "3_FromMemory_ADPCM.wem";
    const std::string& mediaName_4 = "4_Streamed_PCM.wem";
    const std::string& mediaName_5_1 = "5.1_Streamed_Sequence_WemOpus.wem";
    const std::string& mediaName_5_2 = "5.2_Streamed_Sequence_WemOpus.wem";
    const std::string& mediaName_5_3 = "5.3_Streamed_Sequence_WemOpus.wem";

    std::vector<EventAndMediaInfo> eventAndMediaInfo = {
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
    , audioObjectID(0)
    , currentAudioInstance(0)
    , selectedEventAndMediaInfo(eventAndMediaInfo[0])
{}

void PageExternalSources::Initialize()
{
    IPage::Initialize();
    MediaFramework::SubscribeToRenderStage(weak_from_this());

    AudioEngine::SoundbankLoad("ExternalSources.bnk");
    audioObjectID = AudioEngine::AudioObjectGetNewID();
    AudioEngine::AudioObjectRegister(audioObjectID);

    Start();
}

void PageExternalSources::Deinitialize()
{
    IPage::Deinitialize();

    if (audioObjectID != AK_INVALID_AUDIO_OBJECT_ID)
    {
        if (currentAudioInstance != AK_INVALID_UNIQUE_ID)
        {
            StopCurrentPlayback();
        }

        //AudioEngine::CancelAllCallbacksForAudioObject(audioObjectID);
        AudioEngine::AudioObjectUnregister(audioObjectID);
    }

    // SDL_DestroyTexture(titleTextTexture);
    // SDL_DestroySurface(titleTextSurface);
}

void PageExternalSources::Start()
{
    SDL_Renderer* renderer;
    MediaFramework::GetRenderer(renderer);

    // TTF_Font* font = TTF_OpenFont(FONT_PATH, fontPointSizeTitle);
    // titleTextSurface = TTF_RenderText_Solid(font, "External Sources", 0, fontColor);
    // titleTextTexture = SDL_CreateTextureFromSurface(renderer, titleTextSurface);
    //
    // TTF_CloseFont(font);

    //AudioEngine::SoundbankLoad("ExternalSources.bnk"); // AUDIO ENGINE: LOAD EXT SOURCES BANK
}
void PageExternalSources::StageExternalSourceList()
{
    for (const auto& info : eventAndMediaInfo)
    {
        const bool bSelected = selectedEventAndMediaInfo.uiLabel == info.uiLabel;
        ImGuiSelectableFlags_ flags =  bSelected ? ImGuiSelectableFlags_Highlight: ImGuiSelectableFlags_None;
        if (ImGui::Selectable(info.uiLabel.c_str(), bSelected, flags))
        {
            selectedEventAndMediaInfo = info;
        }
    }
}

void PageExternalSources::PlayExternalSource(const EventAndMediaInfo& eventAndMedia)
{
    if (bIsPlaying.load()) { return; }

    std::vector<std::filesystem::path> mediaPaths;
    std::vector<std::wstring> mediaPathsWide;
    std::vector<AudioExternalSourceInfo> media;
    std::vector<InMemoryAudioData> newInMemoryData;

    for (const auto& [mediaName, ExtSourceCookieName, codecID, bIsStreamed] : eventAndMedia.mediaInfo)
    {
        AudioExternalSourceInfo newMedia;
        newMedia.idCodec = codecID;
        newMedia.iExternalSrcCookie = AudioEngine::GetAudioIDFromName(ExtSourceCookieName);

        if (bIsStreamed)
        {
            mediaPaths.push_back(AudioEngine::GetExternalSourcesSubFolder().data() + mediaName);
            mediaPathsWide.push_back(mediaPaths.back().wstring());

#ifdef AK_OS_WCHAR // Windows only!
            newMedia.szFile = mediaPathsWide.back().data();
#else
            newMedia.szFile = mediaName.c_str();
#endif
        }
        else
        {
            mediaPaths.push_back(AudioEngine::GetExternalSourcesBasePath().data() + mediaName);

            InMemoryAudioData newInMemoryDataEntry;
            if (!LoadFile(mediaPaths.back(), newInMemoryDataEntry))
            {
                std::cout << std::format("Failed to load file: {} in memory", mediaPaths.back().string())  << std::endl;
                return;
            }

            newInMemoryData.push_back(std::move(newInMemoryDataEntry));
            newMedia.pInMemory = newInMemoryData.back().data();
            newMedia.uiMemorySize = newInMemoryData.back().size();
        }

        media.push_back(newMedia);
    }

    currentAudioInstance = AudioEngine::PlayAudioEvent(eventAndMedia.eventName, audioObjectID,
        AK_EndOfEvent, &ExternalSourceEventCallback, this, media);

    std::lock_guard lock(audioInMemoryMutex);
    for (auto& newEntry: newInMemoryData)
    {
        currentAudioInMemory.emplace(currentAudioInstance, std::move(newEntry));
    }

    bIsPlaying.store(true);
}

void PageExternalSources::StopCurrentPlayback()
{
    if (currentAudioInstance == AK_INVALID_UNIQUE_ID) { return; }
    AudioEngine::StopPlayingAudioInstance(currentAudioInstance);
    currentAudioInstance = AK_INVALID_UNIQUE_ID;
}

void PageExternalSources::ClearUnusedMemory(uint32_t audioInstanceID)
{
    std::lock_guard lock(audioInMemoryMutex);
    currentAudioInMemory.erase(audioInstanceID);
    bIsPlaying.store(false);
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

    constexpr ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoMove;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    const ImVec2 window_pos {
        (viewport->WorkPos.x + viewport->WorkSize.x) * 0.5f,
        (viewport->WorkPos.y + viewport->WorkSize.y) * 0.5f
    };

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(880, 480));
    if (ImGui::Begin("External Sources", nullptr, window_flags))
    {
        if (ImGui::BeginListBox("##External Sources Inner Box", ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing())))
        {
            StageExternalSourceList();
            ImGui::EndListBox();
        }

        ImGui::Spacing();

        ImGui::PushID("ExtSourceButtonPlay");
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.26f, 0.8f, 0.8f)));
        ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.5f);
        if (ImGui::Button("PLAY")) { PlayExternalSource(selectedEventAndMediaInfo); }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);
        ImGui::PopID();

        ImGui::SameLine();

        ImGui::PushID("ExtSourceButtonStop");
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.0f, 0.8f, 0.8f)));
        ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.5f);
        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::Button("STOP")) { StopCurrentPlayback(); }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);
        ImGui::PopID();
    }
    ImGui::End();
}

bool PageExternalSources::CanClose()
{
    return !bIsPlaying.load();
}

void PageExternalSources::ExternalSourceEventCallback(AudioCallbackType type, AudioCallbackInfo* info)
{
    if (!info) { return; }
    const auto page = static_cast<PageExternalSources*>(info->pCookie);
    const auto eventCallbackInfo = static_cast<AkEventCallbackInfo*>(info);
    if (!(page && eventCallbackInfo)) { return; }

    if (type == AK_EndOfEvent)
    {
        page->ClearUnusedMemory(eventCallbackInfo->playingID);
    }
}

#undef FONT_PATH