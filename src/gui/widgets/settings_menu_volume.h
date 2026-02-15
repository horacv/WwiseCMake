#ifndef SETTINGS_MENU_VOLUME_H
#define SETTINGS_MENU_VOLUME_H

#include "widget.h"

class SettingsMenuVolume final : public IGuiWidget
{
	public:
		SettingsMenuVolume();
		void Initialize() override;
		void Stage(std::vector<InputEvent>& outEvents) override;

	private:
		float mMasterVolumeCurrent;
		float mMusicVolumeCurrent;
		float mSFXVolumeCurrent;
};
#endif