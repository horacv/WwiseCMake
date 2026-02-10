#ifndef GUI_H
#define GUI_H

#include "gui_widget.h"

struct SDL_Renderer;
struct SDL_Window;
union SDL_Event;

class GUI
{
    public:
        static GUI& Get();

        static void Initialize(SDL_Window* window, SDL_Renderer* renderer, int windowHeight, int windowWidth);
        static void ProcessEvents(const SDL_Event* event);
        static void Render(SDL_Renderer* renderer, std::vector<GUIEvent>& outEvents);
        static void Destroy();

    private:
        static std::unique_ptr<GUI> sInstance;
        std::unique_ptr<IGuiWidget> mMainMenu;

        GUI();

        void InitializeWidgets() const;

        static void StageWidgets(std::vector<GUIEvent>& outEvents);
};
#endif