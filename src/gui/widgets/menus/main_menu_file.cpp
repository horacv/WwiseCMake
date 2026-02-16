#include "main_menu_file.h"

namespace
{
    constexpr auto LABEL_MENU = "File";
    constexpr auto LABEL_MENU_ITEM_QUIT = "Quit";
    constexpr auto LABEL_MENU_ITEM_QUIT_SHORTCUT = "Escape";
}

MainMenuFile::MainMenuFile() = default;

void MainMenuFile::Initialize()
{
    IWidget::Initialize();
}

void MainMenuFile::Stage(std::vector<InputEvent>& outEvents)
{
    IWidget::Stage(outEvents);

    if (ImGui::BeginMenu(LABEL_MENU))
    {
        ImGui::Separator();
        if (ImGui::MenuItem(LABEL_MENU_ITEM_QUIT, LABEL_MENU_ITEM_QUIT_SHORTCUT))
        {
            outEvents.emplace_back(QuitRequestedEvent());
        }
        ImGui::EndMenu();
    }
}