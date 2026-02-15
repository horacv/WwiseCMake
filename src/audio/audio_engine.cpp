#include "audio_engine.h"

/*******************************************/
/* Required to auto-register codec plugins */
#include <AK/Plugin/AkVorbisDecoderFactory.h>
#include <AK/Plugin/AkOpusDecoderFactory.h>
/*******************************************/

#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

#ifndef AK_OPTIMIZED // The profiler module
    #include <AK/Comm/AkCommunication.h>
#endif

#include <iostream>
#include <cassert>

#include "audio_config.h"

std::unique_ptr<AudioEngine> AudioEngine::sInstance = nullptr;
uint64_t AudioEngine::sNextAudioObjectID = 0;

namespace
{
    void SetAudioStreamingDeviceSettings(AkDeviceSettings& DeviceSettings, const AudioConfig& config)
    {
        DeviceSettings.uIOMemorySize = config.GetInt("DeviceSettings", "uIOMemorySize");
        DeviceSettings.uIOMemoryAlignment = config.GetInt("DeviceSettings", "uIOMemoryAlignment");
        DeviceSettings.uGranularity = config.GetInt("DeviceSettings", "uGranularity");
        DeviceSettings.fTargetAutoStmBufferLength = config.GetFloat("DeviceSettings", "fTargetAutoStmBufferLength");
        DeviceSettings.uMaxConcurrentIO = config.GetInt("DeviceSettings", "uMaxConcurrentIO");
        DeviceSettings.bUseStreamCache = config.GetBool("DeviceSettings", "bUseStreamCache");
        DeviceSettings.uMaxCachePinnedBytes = static_cast<uint32_t>(config.GetInt("DeviceSettings", "uMaxCachePinnedBytes", -1));
    }

    void SetAudioEngineInitSettings(AkInitSettings& InitSettings, const AudioConfig& config)
    {
        // Decided to keep this hard-coded because it's a pain to deal with wchar_t.
        InitSettings.szPluginDLLPath = AKTEXT("plugins/wwise");

        InitSettings.uMaxNumPaths = config.GetInt("AudioEngine", "uMaxNumPaths");
        InitSettings.uCommandQueueSize = config.GetInt("AudioEngine", "uCommandQueueSize");
        InitSettings.bEnableGameSyncPreparation = config.GetBool("AudioEngine", "bEnableGameSyncPreparation");
        InitSettings.uContinuousPlaybackLookAhead = config.GetInt("AudioEngine", "uContinuousPlaybackLookAhead");
        InitSettings.uNumSamplesPerFrame = config.GetInt("AudioEngine", "uNumSamplesPerFrame");
        InitSettings.uMonitorQueuePoolSize = config.GetInt("AudioEngine", "uMonitorQueuePoolSize");
        InitSettings.uCpuMonitorQueueMaxSize = config.GetInt("AudioEngine", "uCpuMonitorQueueMaxSize");

        std::unordered_map<std::string, AkPanningRule> panningRules{
            {"Speakers", AkPanningRule_Speakers},
            {"Headphones", AkPanningRule_Headphones}
        };

        std::unordered_map<std::string, AkChannelConfigType> channelConfigTypes{
                {"Anonymous", AK_ChannelConfigType_Anonymous},
                {"Standard", AK_ChannelConfigType_Standard},
                {"Ambisonic", AK_ChannelConfigType_Ambisonic},
                {"Objects", AK_ChannelConfigType_Objects},
                {"MainMix", AK_ChannelConfigType_UseDeviceMain},
                {"Passthrough", AK_ChannelConfigType_UseDevicePassthrough},
        };

        // Main Output Settings
        AkOutputSettings settingsMainOutput;
        const std::string deviceName =  config.GetString("OutputSettings", "audioDeviceShareset");

        AkPanningRule panningRule = AkPanningRule_Speakers;
        if (const auto it = panningRules.find(config.GetString("OutputSettings", "ePanningRule"));
            it != panningRules.end())
        {
            panningRule = it->second;
        }

        AkChannelConfigType channelConfig = AK_ChannelConfigType_Anonymous;
        if (const auto it = channelConfigTypes.find(config.GetString("OutputSettings", "eConfigType"));
            it != channelConfigTypes.end())
        {
            channelConfig = it->second;
        }

        settingsMainOutput.audioDeviceShareset = deviceName.empty() ? 0 : AK::SoundEngine::GetIDFromString(deviceName.c_str());
        settingsMainOutput.idDevice = config.GetInt("OutputSettings", "idDevice");
        settingsMainOutput.ePanningRule = panningRule;
        settingsMainOutput.channelConfig.uNumChannels = config.GetInt("OutputSettings", "uNumChannels");
        settingsMainOutput.channelConfig.eConfigType = channelConfig;
        settingsMainOutput.channelConfig.uChannelMask = config.GetInt("OutputSettings", "uChannelMask");
        InitSettings.settingsMainOutput = settingsMainOutput;
        // Main Output Settings

        InitSettings.uMaxHardwareTimeoutMs = config.GetInt("AudioEngine", "uCpuMonitorQueueMaxSize");
        InitSettings.bUseSoundBankMgrThread = config.GetBool("AudioEngine", "bUseSoundBankMgrThread");
        InitSettings.bUseLEngineThread = config.GetBool("AudioEngine", "bUseLEngineThread");

        // Floor Plane Settings
        std::unordered_map<std::string, AkFloorPlane> floorPlaneValues{
            {"ZX", AkFloorPlane_Default},
            {"XY", AkFloorPlane_XY},
            {"XZ", AkFloorPlane_XZ},
            {"YZ", AkFloorPlane_YZ}
        };

        AkFloorPlane floorPlane = AkFloorPlane_Default;
        if (const auto it = floorPlaneValues.find(config.GetString("AudioEngine", "eFloorPlane"));
            it != floorPlaneValues.end())
        {
            floorPlane = it->second;
        }

        InitSettings.eFloorPlane = floorPlane;
        // Floor Plane Settings

        InitSettings.fGameUnitsToMeters = config.GetFloat("AudioEngine", "fGameUnitsToMeters");
        InitSettings.uBankReadBufferSize = config.GetInt("AudioEngine", "uBankReadBufferSize");
        InitSettings.fDebugOutOfRangeLimit = config.GetFloat("AudioEngine", "fDebugOutOfRangeLimit");
        InitSettings.bDebugOutOfRangeCheckEnabled = config.GetBool("AudioEngine", "bDebugOutOfRangeCheckEnabled");
        InitSettings.bOfflineRendering = config.GetBool("AudioEngine", "bOfflineRendering");
    }

    void SetPlatformInitSettings(AkPlatformInitSettings& PlatformInitSettings, const AudioConfig& config)
    {
        const std::string& category = "Platform";

        PlatformInitSettings.uSampleRate = config.GetInt(category, "uSampleRate");

#if defined(AK_WIN)
        PlatformInitSettings.uMaxSystemAudioObjects = config.GetInt(category, "uMaxSystemAudioObjects");
#elif  defined(AK_APPLE)
        PlatformInitSettings.uNumSpatialAudioPointSources = config.GetInt(category, "uMaxSystemAudioObjects");
#endif
    }

    void SetMusicSettings(AkMusicSettings& MusicSettings, const AudioConfig& config)
    {
        MusicSettings.fStreamingLookAheadRatio = config.GetFloat("MusicSettings", "fStreamingLookAheadRatio");
    }

    void SetSpatialAudioInitSettings(AkSpatialAudioInitSettings& SpatialAudioSettings, const AudioConfig& config)
    {
        SpatialAudioSettings.uMaxSoundPropagationDepth = config.GetInt("SpatialAudio", "uMaxSoundPropagationDepth");
        SpatialAudioSettings.fMovementThreshold = config.GetFloat("SpatialAudio", "fMovementThreshold");
        SpatialAudioSettings.uNumberOfPrimaryRays = config.GetInt("SpatialAudio", "uNumberOfPrimaryRays");
        SpatialAudioSettings.uMaxReflectionOrder = config.GetInt("SpatialAudio", "uMaxReflectionOrder");
        SpatialAudioSettings.uMaxDiffractionOrder = config.GetInt("SpatialAudio", "uMaxDiffractionOrder");
        SpatialAudioSettings.uMaxDiffractionPaths = config.GetInt("SpatialAudio", "uMaxDiffractionPaths");
        SpatialAudioSettings.uMaxGlobalReflectionPaths = config.GetInt("SpatialAudio", "uMaxGlobalReflectionPaths");
        SpatialAudioSettings.uMaxEmitterRoomAuxSends = config.GetInt("SpatialAudio", "uMaxEmitterRoomAuxSends");
        SpatialAudioSettings.uDiffractionOnReflectionsOrder = config.GetInt("SpatialAudio", "uDiffractionOnReflectionsOrder");
        SpatialAudioSettings.fMaxDiffractionAngleDegrees = config.GetFloat("SpatialAudio", "fMaxDiffractionAngleDegrees");
        SpatialAudioSettings.fMaxPathLength = config.GetFloat("SpatialAudio", "fMaxPathLength");
        SpatialAudioSettings.fCPULimitPercentage = config.GetFloat("SpatialAudio", "fCPULimitPercentage");
        SpatialAudioSettings.fSmoothingConstantMs = config.GetFloat("SpatialAudio", "fSmoothingConstantMs");
        SpatialAudioSettings.uLoadBalancingSpread = config.GetInt("SpatialAudio", "uLoadBalancingSpread");
        SpatialAudioSettings.bEnableGeometricDiffractionAndTransmission = config.GetBool("SpatialAudio", "bEnableGeometricDiffractionAndTransmission");
        SpatialAudioSettings.bCalcEmitterVirtualPosition = config.GetBool("SpatialAudio", "bCalcEmitterVirtualPosition");

        std::unordered_map<std::string, AkTransmissionOperation> transmissionOperationValues{
            {"Max", AkTransmissionOperation_Max},
            {"Add", AkTransmissionOperation_Add},
            {"Multiply", AkTransmissionOperation_Multiply}
        };

        AkTransmissionOperation transmissionOperation = AkTransmissionOperation_Max;
        if (const auto it = transmissionOperationValues.find(config.GetString("SpatialAudio", "eTransmissionOperation"));
            it != transmissionOperationValues.end())
        {
            transmissionOperation = it->second;
        }

        SpatialAudioSettings.eTransmissionOperation = transmissionOperation;
    }

#ifndef AK_OPTIMIZED
    void SetCommunicationSettings(AkCommSettings& CommunicationSettings, const AudioConfig& config)
    {
        const std::string& appName = config.GetString("CommunicationSettings", "szAppNetworkName");
        std::snprintf(CommunicationSettings.szAppNetworkName, sizeof(CommunicationSettings.szAppNetworkName), "%s", appName.c_str());

        CommunicationSettings.bInitSystemLib = config.GetBool("CommunicationSettings", "bInitSystemLib");

        std::unordered_map<std::string, AkCommSettings::AkCommSystem> communicationSystems{
            {"Socket", AkCommSettings::AkCommSystem_Socket},
            {"HTCS", AkCommSettings::AkCommSystem_HTCS},
        };

        AkCommSettings::AkCommSystem communicationSystem = AkCommSettings::AkCommSystem_Socket;
        if (const auto it = communicationSystems.find(config.GetString("CommunicationSettings", ""));
            it != communicationSystems.end())
        {
            communicationSystem = it->second;
        }

        CommunicationSettings.commSystem = communicationSystem;
        CommunicationSettings.ports.uCommand = config.GetInt("CommunicationSettings", "ports.uCommand");
        CommunicationSettings.ports.uDiscoveryBroadcast = config.GetInt("CommunicationSettings", "ports.uDiscoveryBroadcast");
    }
#endif
}

AudioEngine::AudioEngine()
    : mDefaultAudioObject(0)
    , mDefaultAudioListener(0)
{}

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
	if (IsInitialized()) { return true; } // Already Initialized

    AudioEngine& audioEngine = Get();

    AudioConfig config;
    if (!config.LoadConfigFile("config/audio_engine.ini"))
    {
        assert(!"Could not load configuration file");
    }

    AkMemSettings memorySettings{};
    AK::MemoryMgr::GetDefaultSettings(memorySettings);
    if (AK::MemoryMgr::Init(&memorySettings) != AK_Success)
    {
        assert(!"Could not initialize the memory manager.");
    }

    AkStreamMgrSettings streamingSettings{};
    AK::StreamMgr::GetDefaultSettings(streamingSettings);
    if (!AK::StreamMgr::Create(streamingSettings))
    {
        assert(!"Could not create the Streaming Manager");
    }

    if (AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)")) != AK_Success)
    {
        assert(!"Could not set language");
    }

    AkDeviceSettings streamingDeviceSettings{};
    AK::StreamMgr::GetDefaultDeviceSettings(streamingDeviceSettings);
    SetAudioStreamingDeviceSettings(streamingDeviceSettings, config);
    if (audioEngine.mLowLevelIO.Init(streamingDeviceSettings) != AK_Success)
    {
        assert(!"Could not initialize the Low-Level I/O system");
    }

    if (audioEngine.mLowLevelIO.SetBasePath(AKTEXT("assets/soundbanks/" AUDIO_PLATFORM "/")) != AK_Success)
    {
        assert(!"Failed setting the Soundbanks base path");
    }

    AkInitSettings initSettings{};
    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    SetAudioEngineInitSettings(initSettings, config);
    AkPlatformInitSettings platformInitSettings{};
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
    SetPlatformInitSettings(platformInitSettings, config);
    if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
    {
        assert(!"Could not initialize the Sound Engine.");
    }

    AkMusicSettings musicSettings{};
    AK::MusicEngine::GetDefaultInitSettings(musicSettings);
    SetMusicSettings(musicSettings, config);
    if (AK::MusicEngine::Init(&musicSettings) != AK_Success)
    {
        assert(!"Could not initialize the Music Engine.");
    }

    AkSpatialAudioInitSettings spatialAudioSettings{};
    SetSpatialAudioInitSettings(spatialAudioSettings, config);
    if (AK::SpatialAudio::Init(spatialAudioSettings) != AK_Success)
    {
        assert(!"Could not initialize Spatial Audio." );
    }

#ifndef AK_OPTIMIZED
    AkCommSettings communicationSettings{};
    AK::Comm::GetDefaultInitSettings(communicationSettings);
    SetCommunicationSettings(communicationSettings, config);
    if (AK::Comm::Init(communicationSettings) != AK_Success)
    {
        assert(!"Could not initialize communication.");
    }
#endif

    if (!SoundbankLoad("Init.bnk"))
    {
        assert(!"Failed to load Init.bnk");
    }

    audioEngine.mDefaultAudioObject = AudioObjectGetNewID();
    audioEngine.mDefaultAudioListener = AudioObjectGetNewID();

    if (!(AudioObjectRegister(audioEngine.mDefaultAudioObject, "Default Object")
        && AudioObjectRegister(audioEngine.mDefaultAudioListener, "Default Listener")))
    {
        assert(!"Failed to register default audio game objects");
    }

    SetDefaultListener(audioEngine.mDefaultAudioListener);

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
    Get().mLowLevelIO.Term();

    if (auto* pStrMngr = AK::IAkStreamMgr::Get())
    {
        pStrMngr->Destroy();
    }

    AK::MemoryMgr::Term();

    std::cout << "Audio Engine Terminated" << std::endl;
}

void AudioEngine::Update()
{
    if (!IsInitialized()) { return; }
    AK::SoundEngine::RenderAudio();
}

bool AudioEngine::IsInitialized()
{
    return AK::SoundEngine::IsInitialized();
}

// Soundbanks

bool AudioEngine::SoundbankLoad(const std::string& bank, const AudioBankType type)
{
    if (!IsInitialized()) { return false; }
    AkBankID bankID;
    return AK::SoundEngine::LoadBank(bank.c_str(), bankID, type) == AK_Success;
}

bool AudioEngine::SoundbankUnload(const std::string& bank, const AudioBankType type)
{
    if (!IsInitialized()) { return false; }
    return AK::SoundEngine::UnloadBank(bank.c_str(), nullptr, type);
}

// Listener

void AudioEngine::SetDefaultListener(const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return; }

    constexpr int numListeners = 1;
    AK::SoundEngine::SetDefaultListeners(&audioObjectID, numListeners);
}

// Audio Objects

uint64_t AudioEngine::AudioObjectGetNewID()
{
    return sNextAudioObjectID++;
}

bool AudioEngine::AudioObjectRegister(const uint64_t audioObjectID, const std::string& name)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::RegisterGameObj(audioObjectID, name.c_str());
    return result == AK_Success;
}

bool AudioEngine::AudioObjectUnregister(const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::UnregisterGameObj(audioObjectID);
    return result == AK_Success;
}

bool AudioEngine::AudioObjectUnregisterAll()
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::UnregisterAllGameObj();
    return result == AK_Success;
}

bool AudioEngine::AudioObjectSetPosition(const uint64_t audioObjectID, const AudioPosition& position)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::SetPosition(audioObjectID, position);
    return result == AK_Success;
}

// Events

uint32_t AudioEngine::PlayAudioEvent(const std::string& eventName, const uint64_t audioObjectID,
    const AudioCallbackType callbackType, const AudioCallbackFunc callback, void* callbackCookie)
{
    const AkGameObjectID ID = audioObjectID <= 0 || audioObjectID == AK_INVALID_GAME_OBJECT ? Get().mDefaultAudioObject : audioObjectID;

    AudioPosition position;
    position.Set({0,0,0},{1,0,0},{0,1,0});
    return PlayAudioEvent(eventName, position, ID, callbackType, callback, callbackCookie);
}

uint32_t AudioEngine::PlayAudioEvent(const std::string& eventName, const AudioPosition& position,
    const uint64_t audioObjectID, const AudioCallbackType callbackType,
    const AudioCallbackFunc callback, void* callbackCookie)
{
    if (!IsInitialized()) { return 0; }
    AK::SoundEngine::SetPosition(audioObjectID, position);
    return AK::SoundEngine::PostEvent(eventName.c_str(), audioObjectID, callbackType, callback, callbackCookie);
}

// Parameters

bool AudioEngine::SetState(const std::string& stateGroup, const std::string& stateValue)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::SetState(stateGroup.c_str(), stateValue.c_str());
    return result == AK_Success;
}

bool AudioEngine::SetSwitch(const std::string& switchGroup, const std::string& switchValue, const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::SetSwitch(switchGroup.c_str(), switchValue.c_str(), audioObjectID);
    return result == AK_Success;
}

bool AudioEngine::SetParameter(const std::string& parameterName, const float value, const uint64_t audioObjectID,
        const int valueChangeDuration, const AudioCurveInterpolation curveInterpolation, const bool bBypassInternalInterpolation)
{
    if (!IsInitialized()) { return false; }
    const AKRESULT result = AK::SoundEngine::SetRTPCValue(parameterName.c_str(), value,
        audioObjectID, valueChangeDuration, curveInterpolation, bBypassInternalInterpolation);
    return result == AK_Success;
}

bool AudioEngine::GetParameter(const std::string& parameterName, float& outValue, const AudioParameterType inParameterType,
    AudioParameterType& outParameterType, const uint64_t inAudioObjectID, const uint32_t inEventInstanceID)
{
    if (!IsInitialized()) { return false; }
    AudioParameterType type = inParameterType;
    const AKRESULT result = AK::SoundEngine::Query::GetRTPCValue(parameterName.c_str(),
        inAudioObjectID, inEventInstanceID, outValue, type);

    outParameterType = type;
    return result == AK_Success;
}
