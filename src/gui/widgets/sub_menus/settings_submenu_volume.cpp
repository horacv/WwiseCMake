/*
 * This menu provides master, music, and SFX volume controls.
 * It demonstrates how to get and set global Real-Time Parameter Control (RTPC) values in the Wwise Engine.
 *
 * In SettingsSubMenuVolume::Initialize, the GUI sliders are initialized with the default values in the soundbanks.
 * In SettingsSubMenuVolume::Stage, the Wwise parameters are updated in real-time as the user adjusts the sliders.
 */

#include "settings_submenu_volume.h"

#include "audio/audio_engine.h"

namespace
{
	constexpr int VOLUME_MIN = 0;
	constexpr int VOLUME_MAX = 100;
	constexpr auto SLIDER_RESOLUTION = "%.1f";

	constexpr auto LABEL_TOP_SEPARATOR = "Volume";
	constexpr auto LABEL_SLIDER_MASTER = "Master Volume";
	constexpr auto LABEL_SLIDER_MUSIC = "Music Volume";
	constexpr auto LABEL_SLIDER_SFX = "SFX Volume";
	constexpr auto LABEL_SLIDER_VO = "VO Volume";

	constexpr auto PARAM_MASTER_VOLUME = "Volume_Master";
	constexpr auto PARAM_MUSIC_VOLUME = "Volume_MX";
	constexpr auto PARAM_SFX_VOLUME = "Volume_SX";
	constexpr auto PARAM_VO_VOLUME = "Volume_VO";
}


SettingsSubMenuVolume::SettingsSubMenuVolume()
: mMasterVolumeCurrent(VOLUME_MAX)
, mMusicVolumeCurrent(VOLUME_MAX)
, mSFXVolumeCurrent(VOLUME_MAX)
, mVOVolumeCurrent(VOLUME_MAX)
{}

void SettingsSubMenuVolume::Initialize()
{
	AudioParameterType outParamTemp;
	AudioEngine::GetParameter(PARAM_MASTER_VOLUME, mMasterVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	AudioEngine::GetParameter(PARAM_MUSIC_VOLUME, mMusicVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	AudioEngine::GetParameter(PARAM_SFX_VOLUME, mSFXVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	AudioEngine::GetParameter(PARAM_VO_VOLUME, mVOVolumeCurrent,
		AudioParameterType::RTPCValue_Default, outParamTemp);
	IWidget::Initialize();
}

void SettingsSubMenuVolume::Stage(std::vector<InputEvent>& outEvents)
{
	IWidget::Stage(outEvents);

	if (ImGui::BeginMenu(LABEL_TOP_SEPARATOR))
	{
		ImGui::Separator();

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

		if (ImGui::SliderFloat(LABEL_SLIDER_VO, &mVOVolumeCurrent, VOLUME_MIN, VOLUME_MAX, SLIDER_RESOLUTION))
		{
			AudioEngine::SetParameter(PARAM_VO_VOLUME, mVOVolumeCurrent);
		}

		ImGui::EndMenu();
	}
}
