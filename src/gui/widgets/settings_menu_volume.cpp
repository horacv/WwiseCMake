/*
 * This menu provides master, music, and SFX volume controls.
 * It demonstrates how to get and set global Real-Time Parameter Control (RTPC) values in the Wwise Engine.
 *
 * In SettingsMenuVolume::Initialize, the GUI sliders are initialized with the default values in the soundbanks.
 * In SettingsMenuVolume::Stage, the Wwise parameters are updated in real-time as the user adjusts the sliders.
 */

#include "settings_menu_volume.h"

#include "audio/audio_engine.h"

constexpr int VOLUME_MIN = 0;
constexpr int VOLUME_MAX = 100;
constexpr auto SLIDER_RESOLUTION = "%.1f";

constexpr auto LABEL_TOP_SEPARATOR = "Volume";
constexpr auto LABEL_SLIDER_MASTER = "Master Volume";
constexpr auto LABEL_SLIDER_MUSIC = "Music Volume";
constexpr auto LABEL_SLIDER_SFX = "SFX Volume";

constexpr auto PARAM_MASTER_VOLUME = "Volume_Master";
constexpr auto PARAM_MUSIC_VOLUME = "Volume_MX";
constexpr auto PARAM_SFX_VOLUME = "Volume_SX";

SettingsMenuVolume::SettingsMenuVolume()
: mMasterVolumeCurrent(VOLUME_MAX)
, mMusicVolumeCurrent(VOLUME_MAX)
, mSFXVolumeCurrent(VOLUME_MAX)
{}

void SettingsMenuVolume::Initialize()
{
	AudioParameterType outParamTemp;
	AudioEngine::GetParameter(PARAM_MASTER_VOLUME, mMasterVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	AudioEngine::GetParameter(PARAM_MUSIC_VOLUME, mMusicVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	AudioEngine::GetParameter(PARAM_SFX_VOLUME, mSFXVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	IGuiWidget::Initialize();
}

void SettingsMenuVolume::Stage(std::vector<InputEvent>& outEvents)
{
	IGuiWidget::Stage(outEvents);

	ImGui::SeparatorText(LABEL_TOP_SEPARATOR);

	if (ImGui::SliderFloat(LABEL_SLIDER_MASTER, &mMasterVolumeCurrent, VOLUME_MIN, VOLUME_MAX, SLIDER_RESOLUTION))
	{
		AudioEngine::SetParameter(PARAM_MASTER_VOLUME, mMasterVolumeCurrent);
	}

	if (ImGui::SliderFloat(LABEL_SLIDER_MUSIC, &mMusicVolumeCurrent, VOLUME_MIN, VOLUME_MAX, SLIDER_RESOLUTION))
	{
		AudioEngine::SetParameter(PARAM_MUSIC_VOLUME, mMusicVolumeCurrent);
	}

	if (ImGui::SliderFloat(LABEL_SLIDER_SFX, &mSFXVolumeCurrent, VOLUME_MIN, VOLUME_MAX, SLIDER_RESOLUTION))
	{
		AudioEngine::SetParameter(PARAM_SFX_VOLUME, mSFXVolumeCurrent);
	}
}
