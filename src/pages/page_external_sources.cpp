#include "page_external_sources.h"

#include <ranges>

#include "media/media_framework.h"

#define FONT_PATH "assets/fonts/arial.ttf"

namespace
{
    constexpr float fontPointSizeTitle = 96;
    constexpr float fontPointSizeBeatCounter = 40;
    constexpr SDL_Color fontColor = { 255, 255, 255, 255 };

    std::vector<ExternalSourceInfo> externalSourcesInfo = {
        {"assets/external_sources/MyExternalSource1.wem","MyExternalSource1.wem - From Memory - CodecID: wem opus", AKCODECID_AKOPUS_WEM, false},
        {"MyExternalSource2.wem","MyExternalSource2.wem - Streamed  - CodecID: Vorbis", AKCODECID_VORBIS},
        {"MyExternalSource3.wav","MyExternalSource3.wav - Streamed - CodecID: ADPCM", AKCODECID_ADPCM},
        {"MyExternalSource4.wav","MyExternalSource4.wav - Streamed - CodecID: PCM Wav", AKCODECID_PCM}
    };
}


PageExternalSources::PageExternalSources()
    : titleTextSurface(nullptr)
    , titleTextTexture(nullptr)
    , audioObjectID(0)
    , selectedExternalSourceData(externalSourcesInfo[0])
{}

void PageExternalSources::Initialize()
{
    IPage::Initialize();
    MediaFramework::SubscribeToRenderStage(weak_from_this());

    audioObjectID = AudioEngine::AudioObjectGetNewID();
    AudioEngine::AudioObjectRegister(audioObjectID);

    Start();
}
void PageExternalSources::Deinitialize()
{
    IPage::Deinitialize();

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
    for (const auto& externalSource : externalSourcesInfo)
    {
        const bool bSelected = selectedExternalSourceData.name == externalSource.name;
        if (ImGui::Selectable(externalSource.description.c_str(), bSelected,
            bSelected ? ImGuiSelectableFlags_Highlight: ImGuiSelectableFlags_None))
        {
            selectedExternalSourceData = externalSource;
        }
    }
}

void PageExternalSources::PlayExternalSource(const ExternalSourceInfo& info)
{
    AudioExternalSourceInfo _info;
    _info.idCodec = info.codecID;

    if (info.bIsStreamed)
    {
        std::string name = info.name;
        _info.szFile = name.data();
    }
    else
    {
        if (!LoadFile(info.name, currentAudioInMemory)) { return; }
        _info.pInMemory = currentAudioInMemory.data();
        _info.uiMemorySize = currentAudioInMemory.size();
    }

    const std::vector sources = { _info };
    AudioEngine::PlayAudioEvent("EventName", audioObjectID, AK_EndOfEvent, nullptr, this, sources);
    std::cout << info.name << std::endl;
}

void PageExternalSources::StopCurrentPlayback()
{
    std::cout << "STOP"  << std::endl;
}

bool PageExternalSources::LoadFile(const std::filesystem::path& path, std::vector<std::byte>& outData)
{
    outData.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file) { return false; }

    outData.resize(std::filesystem::file_size(path));
    file.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(outData.size()));

    return outData.size() > 0;
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
    ImGui::SetNextWindowSize(ImVec2(640,480));
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
        if (ImGui::Button("PLAY")) { PlayExternalSource(selectedExternalSourceData); }
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

#undef FONT_PATH