#ifndef SETTINGS_MENU_VOLUME_H
#define SETTINGS_MENU_VOLUME_H

#include "gui/widgets//widget.h"

class SettingsSubMenuVolume final : public IWidget
{
	public:
		SettingsSubMenuVolume();
		~SettingsSubMenuVolume() override = default;

		void Initialize() override;
		void Stage(std::vector<InputEvent>& outEvents) override;

	private:
		float mMasterVolumeCurrent;
		float mMusicVolumeCurrent;
		float mSFXVolumeCurrent;
		float mVOVolumeCurrent;
};
#endif