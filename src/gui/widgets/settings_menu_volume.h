#ifndef SETTINGS_MENU_VOLUME_H
#define SETTINGS_MENU_VOLUME_H

#include "gui/gui_widget.h"

class SettingsMenuVolume final : public IGuiWidget
{
	public:
		SettingsMenuVolume();
		void Initialize() override;
		void Stage(std::vector<GUIEvent>& outEvents) override;

	private:
		float mMasterVolumeCurrent;
		float mMusicVolumeCurrent;
		float mSFXVolumeCurrent;
};
#endif