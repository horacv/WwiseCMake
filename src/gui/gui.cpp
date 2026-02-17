#include "gui.h"

#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "media/media_framework.h"
#include "media/media_framework_data.h"
#include "widgets/menus/main_menu.h"

std::unique_ptr<GUI> GUI::sInstance = nullptr;
bool GUI::bRenderImGuiDemoWindow = false;

GUI::GUI()
: mMainMenu(std::make_unique<MainMenu>())
, bIsInitialized(false)
{
    if (const char* envVar = SDL_GetEnvironmentVariable(SDL_GetEnvironment(),"IMGUI_DEMO"))
    {
        bRenderImGuiDemoWindow = envVar[0] == '1';
    }
}

GUI& GUI::Get()
{
    if (!sInstance)
    {
        sInstance = std::unique_ptr<GUI>(new GUI());
    }
    return *sInstance;
}

bool GUI::Initialize()
{
    SDL_Renderer* renderer; SDL_Window* window;
    if (!(MediaFramework::GetRenderer(renderer) && MediaFramework::GetWindow(window)))
    {
        assert(!"Failed to retrieve renderer or window");
    }

    const MediaWindowSettings& windowSettings = MediaFramework::GetCurrentWindowSettings();

    IMGUI_CHECKVERSION();
    if (!(ImGui::CreateContext()
        && ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)
        && ImGui_ImplSDLRenderer3_Init(renderer)))
    {
        assert(!"Failed to initialize ImGui context");
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2(static_cast<float>(windowSettings.width), static_cast<float>(windowSettings.height));

    auto& instance = Get();
    instance.InitializeWidgets();
    instance.bIsInitialized = true;

    return true;
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

void GUI::RenderClear()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GUI::InitializeWidgets() const
{
    mMainMenu->Initialize();
}

void GUI::StageWidgets(std::vector<InputEvent>& outEvents)
{
    // Activate this to render the super useful ImGui demo window
    // with IMGUI_DEMO=1 as an environment variable
    if (bRenderImGuiDemoWindow) { ImGui::ShowDemoWindow(); }

    const GUI& instance = Get();
    instance.mMainMenu->Stage(outEvents);
}

void GUI::RenderStage(std::vector<InputEvent>& outEvents)
{
    StageWidgets(outEvents);

    ImGui::Render();
    SDL_Renderer* renderer;
    MediaFramework::GetRenderer(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void GUI::Terminate()
{
    const GUI& instance = Get();
    assert(instance.bIsInitialized && "Trying to terminate uninitialized GUI");

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

bool GUI::IsInitialized()
{
    const GUI& guiInstance = Get();
    return guiInstance.bIsInitialized;
}
