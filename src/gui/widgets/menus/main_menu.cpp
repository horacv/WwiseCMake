#include "main_menu.h"

#include "main_menu_about.h"
#include "main_menu_file.h"
#include "main_menu_options.h"
#include "main_menu_pages.h"
#include "main_menu_settings.h"

MainMenu::MainMenu()
: mFileMenu(std::make_unique<MainMenuFile>())
, mPagesMenu(std::make_unique<MainMenuPages>())
, mSettingsMenu(std::make_unique<MainMenuSettings>())
, mOptionsMenu(std::make_unique<MainMenuOptions>())
, mAboutMenu(std::make_unique<MainMenuAbout>())
{}

void MainMenu::Initialize()
{
	IWidget::Initialize();

	mFileMenu->Initialize();
	mPagesMenu->Initialize();
	mSettingsMenu->Initialize();
	mOptionsMenu->Initialize();
	mAboutMenu->Initialize();
}

void MainMenu::Stage(std::vector<InputEvent>& outEvents)
{
	IWidget::Stage(outEvents);

	if (ImGui::BeginMainMenuBar())
	{
		mFileMenu->Stage(outEvents);
		mPagesMenu->Stage(outEvents);
		mSettingsMenu->Stage(outEvents);
		mOptionsMenu->Stage(outEvents);
		mAboutMenu->Stage(outEvents);

		ImGui::EndMainMenuBar();
	}
}
