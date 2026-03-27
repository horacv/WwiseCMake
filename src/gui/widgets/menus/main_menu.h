#ifndef WWISECMAKE_MAIN_MENU_H
#define WWISECMAKE_MAIN_MENU_H

#include "gui/widgets/widget.h"

class MainMenu final : public IWidget
{
	public:
		MainMenu();
		~MainMenu() override = default;

		void Initialize() override;
		void Stage(std::vector<InputEvent>& outEvents) override;

	private:
		std::unique_ptr<IWidget> mFileMenu;
		std::unique_ptr<IWidget> mPagesMenu;
		std::unique_ptr<IWidget> mSettingsMenu;
		std::unique_ptr<IWidget> mOptionsMenu;
		std::unique_ptr<IWidget> mAboutMenu;
};
#endif