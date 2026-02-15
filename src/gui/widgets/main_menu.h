#ifndef WWISECMAKE_MAIN_MENU_H
#define WWISECMAKE_MAIN_MENU_H

#include "widget.h"

class SettingsMenuVolume;

class MainMenu final : public IGuiWidget
{
	public:
		MainMenu();
		~MainMenu() override;

		void Initialize() override;
		void Stage(std::vector<InputEvent>& outEvents) override;

	private:
		std::unique_ptr<SettingsMenuVolume> mSettingsVolumeMenu;
};
#endif