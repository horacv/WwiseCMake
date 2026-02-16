#ifndef MAIN_MENU_ABOUT_H
#define MAIN_MENU_ABOUT_H

#include "gui/widgets/widget.h"

class MainMenuAbout : public IWidget
{
public:
    MainMenuAbout();
    ~MainMenuAbout() override = default;

    void Initialize() override;
    void Stage(std::vector<InputEvent>& outEvents) override;
};

#endif