#include "main_menu_settings.h"

#include "gui/widgets/sub_menus/settings_submenu_volume.h"

namespace
{
	constexpr auto LABEL_MENU = "Settings";
}

MainMenuSettings::MainMenuSettings()
: mVolumeSettingsMenu(std::make_unique<SettingsSubMenuVolume>())
{}

void MainMenuSettings::Initialize()
{
	IWidget::Initialize();

	mVolumeSettingsMenu->Initialize();
}

void MainMenuSettings::Stage(std::vector<InputEvent>& outEvents)
{
	IWidget::Stage(outEvents);

	if (ImGui::BeginMenu(LABEL_MENU))
	{
		ImGui::Separator();
		mVolumeSettingsMenu->Stage(outEvents);

		ImGui::EndMenu();
	}
}
