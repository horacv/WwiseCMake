#ifndef GUI_H
#define GUI_H

#include "widgets/widget.h"

struct MediaWindowSettings;
struct SDL_Renderer;
struct SDL_Window;
union SDL_Event;

class GUI
{
    public:
        static GUI& Get();

        static bool Initialize();
        static void ProcessEvents(const SDL_Event* event);
        static void RenderStage(std::vector<InputEvent>& outEvents);
        static void RenderClear();
        static void Terminate();

        static bool IsInitialized();

    private:
        static std::unique_ptr<GUI> sInstance;
        std::unique_ptr<IWidget> mMainMenu;
        bool bIsInitialized;

        static bool bRenderImGuiDemoWindow;

        GUI();
        void InitializeWidgets() const;
        static void StageWidgets(std::vector<InputEvent>& outEvents);
};
#endif