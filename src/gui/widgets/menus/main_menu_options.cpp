#include "main_menu_options.h"

namespace
{
    constexpr auto LABEL_MENU = "Options";
    constexpr auto LABEL_MENU_ITEM_TOGGLE_AUDIO_INFO = "Toggle Overlay: Audio Info";
}

MainMenuOptions::MainMenuOptions() = default;

void MainMenuOptions::Initialize()
{
    IWidget::Initialize();
}

void MainMenuOptions::Stage(std::vector<InputEvent>& outEvents)
{
    IWidget::Stage(outEvents);

    if (ImGui::BeginMenu(LABEL_MENU))
    {
        ImGui::Separator();
        if (ImGui::MenuItem(LABEL_MENU_ITEM_TOGGLE_AUDIO_INFO))
        {
            outEvents.emplace_back(ToggleAudioInfoOverlayEvent());
        }

        ImGui::EndMenu();
    }
}

