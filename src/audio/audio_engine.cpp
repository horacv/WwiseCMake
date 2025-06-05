#include "audio_engine.h"

/*******************************************/
/* Required to auto-register codec plugins */
#include <AK/Plugin/AkVorbisDecoderFactory.h>
#include <AK/Plugin/AkOpusDecoderFactory.h>
/*******************************************/

#include <iostream>
#include <cassert>

std::unique_ptr<AudioEngine> AudioEngine::sInstance = nullptr;
uint64_t AudioEngine::nextAudioObjectID = 0;

AudioEngine::AudioEngine()
    : defaultAudioObject(0)
    , defaultAudioListener(0)
    , mMemorySettings()
    , mStreamingDeviceSettings()
    , mInitSettings()
    , mPlatformInitSettings()
    , mMusicSettings()
{}

uint64_t AudioEngine::GetNewAudioObjectID()
{
    return nextAudioObjectID++;
}

AudioEngine& AudioEngine::Get()
{
	if (!sInstance)
	{
		sInstance = std::unique_ptr<AudioEngine>(new AudioEngine());
	}
	return *sInstance;
}

bool AudioEngine::Initialize()
{
	if (IsInitialized()) { return true; }

    AK::MemoryMgr::GetDefaultSettings(mMemorySettings);
    if (AK::MemoryMgr::Init(&mMemorySettings) != AK_Success)
    {
        assert(!"Could not initialize the memory manager.");
    }

    AK::StreamMgr::GetDefaultSettings(mStreamingSettings);
    if (!AK::StreamMgr::Create(mStreamingSettings))
    {
        assert(!"Could not create the Streaming Manager");
    }

    if (AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)")) != AK_Success)
    {
        assert(!"Could not set language");
    }

    AK::StreamMgr::GetDefaultDeviceSettings(mStreamingDeviceSettings);
    if (mLowLevelIO.Init(mStreamingDeviceSettings) != AK_Success)
    {
        assert(!"Could not initialize the Low-Level I/O system");
    }

    if (mLowLevelIO.SetBasePath(AKTEXT("assets/soundbanks/" AUDIO_PLATFORM "/")) != AK_Success)
    {
        assert(!"Failed setting the Soundbanks base path");
    }

    AK::SoundEngine::GetDefaultInitSettings(mInitSettings);
    SetInitSettings();
    AK::SoundEngine::GetDefaultPlatformInitSettings(mPlatformInitSettings);
    if (AK::SoundEngine::Init(&mInitSettings, &mPlatformInitSettings) != AK_Success)
    {
        assert(!"Could not initialize the Sound Engine.");
    }

    AK::MusicEngine::GetDefaultInitSettings(mMusicSettings);
    if (AK::MusicEngine::Init(&mMusicSettings) != AK_Success)
    {
        assert(!"Could not initialize the Music Engine.");
    }

    if (AK::SpatialAudio::Init(mSpatialAudioSettings) != AK_Success)
    {
        assert(!"Could not initialize Spatial Audio." );
    }

#ifndef AK_OPTIMIZED
    AK::Comm::GetDefaultInitSettings(mCommunicationSettings);
    if (AK::Comm::Init(mCommunicationSettings) != AK_Success)
    {
        assert(!"Could not initialize communication.");
    }
#endif

    if (!LoadSoundBank("Init.bnk"))
    {
        assert(!"Failed to load Init.bnk");
    }

    defaultAudioObject = GetNewAudioObjectID();
    defaultAudioListener = GetNewAudioObjectID();

    if (!(RegisterAudioObject(defaultAudioObject, "Default Object")
        && RegisterAudioObject(defaultAudioListener, "Default Listener")))
    {
        assert(!"Failed to register default audio game objects");
    }

    SetDefaultListener(defaultAudioListener);

    std::cout << "Audio Engine Initialized" << std::endl;

    return IsInitialized();
}

void AudioEngine::Terminate()
{
    if (!IsInitialized()) { return; }

#ifndef AK_OPTIMIZED
    AK::Comm::Term();
#endif

    AK::MusicEngine::Term();
    AK::SoundEngine::Term();
    mLowLevelIO.Term();

    if (auto* pStrMngr = AK::IAkStreamMgr::Get())
    {
        pStrMngr->Destroy();
    }

    AK::MemoryMgr::Term();

    std::cout << "Audio Engine Terminated" << std::endl;
}

void AudioEngine::RenderAudio()
{
    if (!IsInitialized()) { return; }
    AK::SoundEngine::RenderAudio();
}

bool AudioEngine::IsInitialized()
{
    return AK::SoundEngine::IsInitialized();
}

void AudioEngine::SetDefaultListener(const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return; }

    constexpr int numListeners = 1;
    AK::SoundEngine::SetDefaultListeners(&audioObjectID, numListeners);
}

bool AudioEngine::LoadSoundBank(const std::string& bank)
{
    if (!IsInitialized()) { return false; }
    AkBankID bankID;
    return AK::SoundEngine::LoadBank(bank.c_str(), bankID) == AK_Success;
}

bool AudioEngine::RegisterAudioObject(const uint64_t audioObjectID, const std::string& name)
{
    return AK::SoundEngine::RegisterGameObj(audioObjectID, name.c_str()) == AK_Success;
}

uint32_t AudioEngine::PlayAudioEvent(const std::string& eventName, const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return 0; }
    return AK::SoundEngine::PostEvent(eventName.c_str(), audioObjectID <= 0 ? Get().defaultAudioObject : audioObjectID);
}

void AudioEngine::SetInitSettings()
{
    mInitSettings.szPluginDLLPath = AKTEXT("plugins/wwise"); // Where to look for the dynamic library plugins
}
