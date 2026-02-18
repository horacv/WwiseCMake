#ifndef AUDIO_INFO_OVERLAY_H
#define AUDIO_INFO_OVERLAY_H

#include "gui/widgets/widget.h"

class AudioInfoOverlay : public IWidget
{
public:
    AudioInfoOverlay();
    ~AudioInfoOverlay() override = default;

    void Initialize() override;
    void Stage(std::vector<InputEvent>& outEvents) override;
};

#endif