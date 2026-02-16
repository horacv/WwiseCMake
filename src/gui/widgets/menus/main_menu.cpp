#include "main_menu.h"

#include "main_menu_about.h"
#include "main_menu_file.h"
#include "main_menu_settings.h"

MainMenu::MainMenu()
: mFileMenu(std::make_unique<MainMenuFile>())
, mSettingsMenu(std::make_unique<MainMenuSettings>())
, mAboutMenu(std::make_unique<MainMenuAbout>())
{}

void MainMenu::Initialize()
{
	IWidget::Initialize();

	mFileMenu->Initialize();
	mSettingsMenu->Initialize();
	mAboutMenu->Initialize();
}

void MainMenu::Stage(std::vector<InputEvent>& outEvents)
{
	IWidget::Stage(outEvents);

	if (ImGui::BeginMainMenuBar())
	{
		mFileMenu->Stage(outEvents);
		mSettingsMenu->Stage(outEvents);
		mAboutMenu->Stage(outEvents);

		ImGui::EndMainMenuBar();
	}
}
