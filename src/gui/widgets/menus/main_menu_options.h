#ifndef MAIN_MENU_OPTIONS_H
#define MAIN_MENU_OPTIONS_H

#include "gui/widgets/widget.h"

class MainMenuOptions : public IWidget
{
    public:
        MainMenuOptions();
        ~MainMenuOptions() override = default;

        void Initialize() override;
        void Stage(std::vector<InputEvent>& outEvents) override;
};

#endif