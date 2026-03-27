#include "main_menu_pages.h"

namespace
{
    constexpr auto LABEL_MENU = "Pages";
    constexpr auto LABEL_MENU_ITEM_COVER = "Cover";
    constexpr auto LABEL_MENU_ITEM_EXT_SOURCES = "External Sources";
}

MainMenuPages::MainMenuPages()
    : mCurrentPage(LABEL_MENU_ITEM_COVER)
{}

void MainMenuPages::Initialize()
{
    IWidget::Initialize();
}

void MainMenuPages::Stage(std::vector<InputEvent>& outEvents)
{
    IWidget::Stage(outEvents);

    if (ImGui::BeginMenu(LABEL_MENU))
    {
        ImGui::Separator();
        if (ImGui::MenuItem(LABEL_MENU_ITEM_COVER, nullptr, false, mCurrentPage != LABEL_MENU_ITEM_COVER))
        {
            outEvents.emplace_back(OpenPageEvent(LABEL_MENU_ITEM_COVER));
            mCurrentPage = LABEL_MENU_ITEM_COVER;
        }
        else if (ImGui::MenuItem(LABEL_MENU_ITEM_EXT_SOURCES, nullptr, false, mCurrentPage != LABEL_MENU_ITEM_EXT_SOURCES))
        {
            outEvents.emplace_back(OpenPageEvent(LABEL_MENU_ITEM_EXT_SOURCES));
            mCurrentPage = LABEL_MENU_ITEM_EXT_SOURCES;
        }
        ImGui::EndMenu();
    }
}