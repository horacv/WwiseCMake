#ifndef PAGE_COVER_H
#define PAGE_COVER_H

#include "page.h"

#include "audio/audio_engine.h"

class PageCover : public IPage
{
public:
    PageCover();
    void Initialize() override;
    void Start() override;
    void RenderStage() override;

private:
    SDL_Surface* titleTextSurface;
    SDL_Texture* titleTextTexture;

    mutable std::mutex mMusicDataMutex;
    int mCurrentMusicBar;
    int mCurrentMusicBeat;

    /**
    * Audio Event Callback.
    * Refer to: https://www.audiokinetic.com/en/public-library/2024.1.6_8842/?source=SDK&id=soundengine_music_callbacks.html
    * "The callbacks are done from the sound engine's main thread.
    * This means that your application should gather all the information it needs from the notification and return immediately.
    * If any processing needs to be done, it should be performed in a separate thread after the relevant information has been copied from the notification."
    */
    static void MusicEventCallback(AudioCallbackType type, AudioCallbackInfo* info);

    void SetCurrentMusicBarAndBeat(int bar, int beat);
    std::pair<int, int> GetCurrentMusicBarAndBeat() const;
};


#endif