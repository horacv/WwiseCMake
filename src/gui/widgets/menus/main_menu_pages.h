#ifndef MAIN_MENU_PAGES_H
#define MAIN_MENU_PAGES_H

#include "gui/widgets/widget.h"

class MainMenuPages : public IWidget
{
public:
    MainMenuPages();
    ~MainMenuPages() override = default;

    void Initialize() override;
    void Stage(std::vector<InputEvent>& outEvents) override;

private:
    std::string mCurrentPage;
};

#endif