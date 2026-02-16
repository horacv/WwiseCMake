#ifndef MAIN_MENU_FILE_H
#define MAIN_MENU_FILE_H

#include "gui/widgets/widget.h"

class MainMenuFile : public IWidget
{
public:
    MainMenuFile();
    ~MainMenuFile() override = default;

    void Initialize() override;
    void Stage(std::vector<InputEvent>& outEvents) override;
};

#endif