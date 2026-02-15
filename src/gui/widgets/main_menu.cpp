#include "main_menu.h"

#include "settings_menu_volume.h"

constexpr auto LABEL_MAIN_MENU = "Main Menu";
constexpr auto LABEL_TOP_SEPARATOR = "WwiseCMake";
constexpr auto LABEL_MENU_SETTINGS = "Settings";
constexpr auto LABEL_MENU_SETTINGS_AUDIO = "Audio";
constexpr auto LABEL_MENU_QUIT = "Quit";
constexpr auto LABEL_MENU_QUIT_SHORTCUT = "Escape";

MainMenu::MainMenu()
: mSettingsVolumeMenu(std::make_unique<SettingsMenuVolume>())
{}

MainMenu::~MainMenu() = default;

void MainMenu::Initialize()
{
	mSettingsVolumeMenu->Initialize();
	IGuiWidget::Initialize();
}

void MainMenu::Stage(std::vector<InputEvent>& outEvents)
{
	IGuiWidget::Stage(outEvents);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(LABEL_MAIN_MENU))
		{
			ImGui::SeparatorText(LABEL_TOP_SEPARATOR);
			if (ImGui::BeginMenu(LABEL_MENU_SETTINGS))
			{
				if (ImGui::BeginMenu(LABEL_MENU_SETTINGS_AUDIO))
				{
					mSettingsVolumeMenu->Stage(outEvents);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem(LABEL_MENU_QUIT, LABEL_MENU_QUIT_SHORTCUT))
			{
				outEvents.emplace_back(QuitRequestedEvent());
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
