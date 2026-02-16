#include "main_menu_about.h"

#include "AK/AkWwiseSDKVersion.h"

namespace
{
    constexpr auto LABEL_MENU = "About";
    constexpr auto LABEL_SEPARATOR_LIBRARIES = "Libraries";
    const std::string TEXT_SDL_VERSION = std::format("SDL: v{}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    const std::string TEXT_SDL_TTF_VERSION = std::format("SDL_ttf: v{}.{}.{}", SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION, SDL_TTF_MICRO_VERSION);
    const std::string TEXT_IMGUI_VERSION = std::format("Dear ImGui: v{}", IMGUI_VERSION);
    const std::string TEXT_WWISE_VERSION = std::format("Wwise SDK: {}.{}", AK_WWISESDK_VERSIONNAME, AK_WWISESDK_VERSION_BUILD);

    constexpr auto LABEL_SEPARATOR_CREDITS = "Credits";
    constexpr auto COLOR_CREDITS = ImVec4(0.7f, 1.f, 0.7f, 1.f);
    constexpr auto TEXT_CREATED_BY = "Created by: Horacio Valdivieso Sotomayor";
    constexpr auto TEXT_COMPANY = "ABOVE NOISE STUDIOS";
    constexpr auto TEXT_EMAIL = "horacio@abovenoisestudios.com";

}

MainMenuAbout::MainMenuAbout() = default;

void MainMenuAbout::Initialize()
{
    IWidget::Initialize();
}

void MainMenuAbout::Stage(std::vector<InputEvent>& outEvents)
{
    IWidget::Stage(outEvents);

    if (ImGui::BeginMenu(LABEL_MENU))
    {
        ImGui::SeparatorText(LABEL_SEPARATOR_LIBRARIES);
        ImGui::TextUnformatted(TEXT_SDL_VERSION.c_str());
        ImGui::TextUnformatted(TEXT_SDL_TTF_VERSION.c_str());
        ImGui::TextUnformatted(TEXT_IMGUI_VERSION.c_str());
        ImGui::TextUnformatted(TEXT_WWISE_VERSION.c_str());
        ImGui::Spacing();

        ImGui::SeparatorText(LABEL_SEPARATOR_CREDITS);
        ImGui::TextColored(COLOR_CREDITS, TEXT_CREATED_BY);
        ImGui::TextColored(COLOR_CREDITS,TEXT_COMPANY);
        ImGui::TextColored(COLOR_CREDITS,TEXT_EMAIL);

        ImGui::EndMenu();
    }
}
