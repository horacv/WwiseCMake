#include "audio_info_overlay.h"

#include "audio/audio_engine.h"

namespace
{
    constexpr auto OVERLAY_NAME = "Audio Info Overlay";

    constexpr auto TEXT_LABEL_SAMPLE_RATE = "Sample Rate: %i";
    constexpr auto TEXT_LABEL_CHANNEL_CONFIG = "Channel Config: %s";
    constexpr auto TEXT_LABEL_NUM_CHANNELS = "Number of Channels: %i";
    constexpr auto TEXT_LABEL_DEFAULT_DEVICE_NAME = "Default Device: %ls";

    constexpr float PADDING = 10.0f;
    constexpr ImVec2 WINDOW_PIVOT {1, 1};
}

AudioInfoOverlay::AudioInfoOverlay() = default;

void AudioInfoOverlay::Initialize()
{
    IWidget::Initialize();
}

void AudioInfoOverlay::Stage(std::vector<InputEvent>& outEvents)
{
    IWidget::Stage(outEvents);

    constexpr ImGuiWindowFlags window_flags =
          ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoMove;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 window_pos {
        viewport->WorkPos.x + viewport->WorkSize.x - PADDING,
        viewport->WorkPos.y + viewport->WorkSize.y - PADDING
    };

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, WINDOW_PIVOT);
    ImGui::SetNextWindowBgAlpha(0.25f);

    if (ImGui::Begin(OVERLAY_NAME, nullptr, window_flags))
    {
        const uint32_t sampleRate = AudioEngine::GetDeviceSampleRate();
        std::wstring defaultDeviceName = L"None";
        std::string channelConfigType = "None";
        uint32_t numberOfChannels = 0;

        AudioEngine::GetDeviceChannelConfigType(channelConfigType, numberOfChannels);
        AudioEngine::GetDefaultAudioDeviceName(defaultDeviceName);

        ImGui::Text(TEXT_LABEL_SAMPLE_RATE, sampleRate);
        ImGui::Text(TEXT_LABEL_CHANNEL_CONFIG, channelConfigType.c_str());
        ImGui::Text(TEXT_LABEL_NUM_CHANNELS, numberOfChannels);
        ImGui::Text(TEXT_LABEL_DEFAULT_DEVICE_NAME, defaultDeviceName.data());
    }
    ImGui::End();
}
