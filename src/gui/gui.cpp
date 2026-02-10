#include "gui.h"

#include "SDL3/SDL_events.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "widgets/main_menu.h"

std::unique_ptr<GUI> GUI::sInstance = nullptr;

GUI::GUI()
: mMainMenu(std::make_unique<MainMenu>())
{}

GUI& GUI::Get()
{
    if (!sInstance)
    {
        sInstance = std::unique_ptr<GUI>(new GUI());
    }
    return *sInstance;
}

void GUI::Initialize(SDL_Window* window, SDL_Renderer* renderer, const int windowHeight, const int windowWidth)
{
    const auto& guiInstance = Get();

    if (window == nullptr || renderer == nullptr)
    {
        assert(!"SDL window or renderer is null");
    }

    IMGUI_CHECKVERSION();
    if (!(ImGui::CreateContext()
        && ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)
        && ImGui_ImplSDLRenderer3_Init(renderer)))
    {
        assert(!"Failed to initialize ImGui context");
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2(static_cast<float>(windowWidth), static_cast<float>(windowHeight));

    guiInstance.InitializeWidgets();
}

void GUI::ProcessEvents(const SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);

    float mousePositionX;
    float mousePositionY;
    const SDL_MouseButtonFlags mouseFlags = SDL_GetMouseState(&mousePositionX, &mousePositionY);
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(mousePositionX, mousePositionY);

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        io.MouseDown[0] = mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT);
        io.MouseDown[1] = mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT);
    }
}

void GUI::InitializeWidgets() const
{
    mMainMenu->Initialize();
}

void GUI::StageWidgets(std::vector<GUIEvent>& outEvents)
{
    //ImGui::ShowDemoWindow(); // Activate this to render the super useful ImGui demo window

    const GUI& guiInstance = Get();
    guiInstance.mMainMenu->Stage(outEvents);
}

void GUI::Render(SDL_Renderer* renderer, std::vector<GUIEvent>& outEvents)
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    StageWidgets(outEvents);

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void GUI::Destroy()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
