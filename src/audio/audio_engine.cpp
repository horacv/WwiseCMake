#include "audio_engine.h"

#include <AK/Acoustics/Common/AkAcousticsTypes.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkOption.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

/*******************************************/
/* Required to auto-register codec plugins */
/* ALWAYS include these after the Sound Engine modules */
#include <AK/Plugin/AkVorbisDecoderFactory.h> // NOLINT
#include <AK/Plugin/AkOpusDecoderFactory.h> // NOLINT
/*******************************************/

#include <iostream>
#include <cassert>

#include "audio_config.h"

namespace
{
    constexpr std::string_view configFilePath = "config/audio_engine.ini";
}

std::unique_ptr<AudioEngine> AudioEngine::sInstance = nullptr;
uint64_t AudioEngine::sNextAudioObjectID = 0;

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
    if (!config.LoadConfigFile(configFilePath.data()))
    {
        assert(!"Could not load configuration file");
    }

#pragma region Init Settings - Stream Manager
    AK::Option::SetP(AkOption_StreamMgr_LowLevelIOHook, nullptr); // No custom low-level IO hook implementation. Add it here if required.
    AK::Option::SetI(AkOption_StreamMgr_IOMemorySize, config.GetInt("StreamingManager", "uIOMemorySize"));
    AK::Option::SetI(AkOption_StreamMgr_IOMemoryAlignment, config.GetInt("StreamingManager", "uIOMemoryAlignment"));
    AK::Option::SetI(AkOption_StreamMgr_Granularity, config.GetInt("StreamingManager", "uGranularity"));
    AK::Option::SetF(AkOption_StreamMgr_AutoStreamTargetBufferLength, config.GetFloat("StreamingManager", "fTargetAutoStmBufferLength"));
    AK::Option::SetI(AkOption_StreamMgr_MaxConcurrentIO, config.GetInt("StreamingManager", "uMaxConcurrentIO"));
    AK::Option::SetI(AkOption_StreamMgr_UseStreamCache, config.GetBool("StreamingManager", "bUseStreamCache"));
    AK::Option::SetI(AkOption_StreamMgr_MaxPinnedCacheSize, config.GetInt("StreamingManager", "uMaxCachePinnedBytes", -1));
#pragma endregion

#pragma region Init Settings - Audio Manager
    AK::Option::SetI(AkOption_AudioMgr_CommandQueueSize, config.GetInt("AudioManager", "uCommandQueueSize"));
    AK::Option::SetI(AkOption_AudioMgr_NumSamplesPerFrame, config.GetInt("AudioManager", "uNumSamplesPerFrame"));
    AK::Option::SetI(AkOption_AudioMgr_MaxHardwareTimeoutMs, config.GetInt("AudioManager", "uMaxHardwareTimeoutMs"));
    AK::Option::SetI(AkOption_AudioMgr_EnableThread, config.GetBool("AudioManager", "bUseLEngineThread"));
    AK::Option::SetI(AkOption_AudioMgr_EnableDebugOutOfRangeCheck, config.GetBool("AudioManager", "bDebugOutOfRangeCheckEnabled"));
    AK::Option::SetF(AkOption_AudioMgr_DebugOutOfRangeLimit, config.GetFloat("AudioManager", "fDebugOutOfRangeLimit"));
    AK::Option::SetI(AkOption_AudioMgr_EnableOfflineRendering, config.GetBool("AudioManager", "bOfflineRendering"));
    AK::Option::SetI(AkOption_AudioMgr_SampleRate, config.GetInt("AudioManager", "uSampleRate"));
#pragma endregion

#pragma region Init Settings - Bank Manager
    AK::Option::SetI(AkOption_BankMgr_EnableManualGameSyncPreparation, config.GetBool("BankManager", "bEnableGameSyncPreparation"));
    AK::Option::SetI(AkOption_BankMgr_EnableThread, config.GetBool("BankManager", "bUseSoundBankMgrThread"));
    AK::Option::SetI(AkOption_BankMgr_BankReadBufferSize, config.GetInt("BankManager", "uBankReadBufferSize"));
#pragma endregion

#pragma region Init Settings - Profiler
    AK::Option::SetI(AkOption_Profiler_MonitorQueuePoolSize, config.GetInt("Profiler", "uMonitorQueuePoolSize"));
    AK::Option::SetI(AkOption_Profiler_CPUMonitorQueueMaxSize, config.GetInt("Profiler", "uCpuMonitorQueueMaxSize"));
#pragma endregion

#pragma region Init Settings - Sound Engine
    const std::string& plugin_path = config.GetString("SoundEngine", "szPluginDLLPath");
    AK::Option::SetS(AkOption_SoundEngine_PluginPath, plugin_path.c_str());
    const std::string& floor_plane = config.GetString("SoundEngine", "eFloorPlane");
    AK::Option::SetS(AkOption_SoundEngine_FloorPlane, floor_plane.c_str());

    AK::Option::SetI(AkOption_SoundEngine_MaxNumPaths, config.GetInt("SoundEngine", "uMaxNumPaths"));
    AK::Option::SetI(AkOption_SoundEngine_ContinuousPlaybackLookAheadMS, config.GetInt("SoundEngine", "uContinuousPlaybackLookAhead"));
    AK::Option::SetF(AkOption_SoundEngine_GameUnitsPerMeters, config.GetFloat("SoundEngine", "fGameUnitsToMeters"));
#pragma endregion

#pragma region Init Settings - Output
    const std::string& deviceName = config.GetString("Output", "audioDeviceShareset");
    AK::Option::SetI(AkOption_Output_AudioDeviceShareset, deviceName.empty() ? 0 : AK::SoundEngine::GetIDFromString(deviceName.c_str()));

    const std::string& panning_rule = config.GetString("Output", "ePanningRule");
    AK::Option::SetS(AkOption_Output_PanningRule, panning_rule.c_str());

    AK::Option::SetI(AkOption_Output_DeviceID, config.GetInt("Output", "idDevice"));
    AK::Option::SetI(AkOption_Output_MaxSystemAudioObjects, config.GetInt("Output", "uMaxSystemAudioObjects"));

#if defined(AK_APPLE)
    //PlatformInitSettings.uNumSpatialAudioPointSources = config.GetInt(category, "uMaxSystemAudioObjects");
#endif

    // CHANNEL CONFIG
    std::unordered_map<std::string, AkChannelConfigType> channelConfigTypes {
            {"Anonymous", AK_ChannelConfigType_Anonymous},
            {"Standard", AK_ChannelConfigType_Standard},
            {"Ambisonic", AK_ChannelConfigType_Ambisonic},
            {"Objects", AK_ChannelConfigType_Objects},
            {"MainMix", AK_ChannelConfigType_UseDeviceMain},
            {"Passthrough", AK_ChannelConfigType_UseDevicePassthrough},
        };

    AkChannelConfigType channelConfigType = AK_ChannelConfigType_Anonymous;
    if (const auto it = channelConfigTypes.find(config.GetString("Output", "eConfigType"));
        it != channelConfigTypes.end())
    {
        channelConfigType = it->second;
    }

    AkChannelConfig channelConfig;
    channelConfig.eConfigType = channelConfigType;
    channelConfig.uNumChannels = config.GetInt("Output", "uNumChannels");
    channelConfig.uChannelMask = config.GetInt("Output", "uChannelMask");
    AK::Option::SetI(AkOption_Output_ChannelConfig, channelConfig.Serialize());
#pragma endregion

#pragma region Init Settings - Acoustics
    AK::Option::SetI(AkOption_Acoustics_MaxSoundPropagationDepth, config.GetInt("Acoustics", "uMaxSoundPropagationDepth"));
    AK::Option::SetF(AkOption_Acoustics_MovementThreshold, config.GetFloat("Acoustics", "fMovementThreshold"));
    AK::Option::SetI(AkOption_Acoustics_NumberOfPrimaryRays, config.GetInt("Acoustics", "uNumberOfPrimaryRays"));
    AK::Option::SetI(AkOption_Acoustics_MaxReflectionOrder, config.GetInt("Acoustics", "uMaxReflectionOrder"));
    AK::Option::SetI(AkOption_Acoustics_MaxDiffractionOrder, config.GetInt("Acoustics", "uMaxDiffractionOrder"));
    AK::Option::SetI(AkOption_Acoustics_MaxDiffractionPaths, config.GetInt("Acoustics", "uMaxDiffractionPaths"));
    AK::Option::SetI(AkOption_Acoustics_MaxGlobalReflectionPaths, config.GetInt("Acoustics", "uMaxGlobalReflectionPaths"));
    AK::Option::SetI(AkOption_Acoustics_MaxEmitterRoomAuxSends, config.GetInt("Acoustics", "uMaxEmitterRoomAuxSends"));
    AK::Option::SetI(AkOption_Acoustics_DiffractionOnReflectionsOrder, config.GetInt("Acoustics", "uDiffractionOnReflectionsOrder"));
    AK::Option::SetF(AkOption_Acoustics_MaxDiffractionAngleDegrees, config.GetFloat("Acoustics", "fMaxDiffractionAngleDegrees"));
    AK::Option::SetF(AkOption_Acoustics_MaxPathLength, config.GetFloat("Acoustics", "fMaxPathLength"));
    AK::Option::SetF(AkOption_Acoustics_CPULimitPercentage, config.GetFloat("Acoustics", "fCPULimitPercentage"));
    AK::Option::SetF(AkOption_Acoustics_SmoothingConstantMs, config.GetFloat("Acoustics", "fSmoothingConstantMs"));
    AK::Option::SetI(AkOption_Acoustics_LoadBalancingSpread, config.GetInt("Acoustics", "uLoadBalancingSpread"));
    AK::Option::SetI(AkOption_Acoustics_EnableGeometricDiffractionAndTransmission, config.GetBool("Acoustics", "bEnableGeometricDiffractionAndTransmission"));
    AK::Option::SetI(AkOption_Acoustics_CalcEmitterVirtualPosition, config.GetBool("Acoustics", "bCalcEmitterVirtualPosition"));

    const std::string& transmission_loss = config.GetString("Acoustics", "eTransmissionOperation");
    AK::Option::SetS(AkOption_Acoustics_TransmissionOperation, transmission_loss.c_str());
#pragma endregion

#pragma region Init Settings - Communication
    const std::string& app_network_name = config.GetString("Communication", "szAppNetworkName");
    AK::Option::SetS(AkOption_Comm_AppNetworkName, app_network_name.c_str());
    const std::string& communication_api = config.GetString("Communication", "commSystem");
    AK::Option::SetS(AkOption_Comm_API, communication_api.c_str());

    AK::Option::SetI(AkOption_Comm_Enable, config.GetBool("Communication", "bInitSystemLib"));
    AK::Option::SetI(AkOption_Comm_CommandPort, config.GetInt("Communication", "ports.uCommanHVd"));
    AK::Option::SetI(AkOption_Comm_DiscoveryPort, config.GetInt("Communication", "ports.uDiscoveryBroadcast"));
#pragma endregion

    if (AK::MemoryMgr::Init() != AK_Success)
    {

        assert(!"Could not initialize the memory manager.");
    }

    if (AK::SoundEngine::Init() != AK_Success)
    {
        assert(!"Could not initialize the Sound Engine.");
    }

    std::string soundbankPath = config.GetString("Defaults", "szSoundbankBasePath", "");
    if (soundbankPath.empty())
    {
        assert(!"Sound Bank Base Path is empty");
    }

    audioEngine.soundbankBasePath =  soundbankPath + "/" + AUDIO_PLATFORM + "/";
    std::string externalSourcesFolderName = config.GetString("Defaults", "szExternalSourcesFolderName", "");
    audioEngine.externalSourcesSubFolder = externalSourcesFolderName + "/";
    audioEngine.externalSourcesBasePath = audioEngine.soundbankBasePath + audioEngine.externalSourcesSubFolder;

    std::filesystem::path soundbankPlatformPath = audioEngine.soundbankBasePath;

#ifdef AK_OS_WCHAR // Windows only - Wide!
    std::wstring path = soundbankPlatformPath.wstring();
#else              // Rest OS - Narrow!
    std::string path = soundbankPlatformPath.string();
#endif

    if (AK::StreamMgr::AddBasePath(path.data()) != AK_Success)
    {
        assert(!"Failed setting the Soundbanks base path");
    }

    std::string language_str_narrow = config.GetString("Defaults", "InitLanguage", "");
#ifdef AK_OS_WCHAR // Windows only - Wide!
    auto size = static_cast<AkUInt32>(language_str_narrow.size() + 1);
    std::wstring language(size, L'\0');
    AKPLATFORM::AkUtf8ToWideChar(language_str_narrow.c_str(), size, language.data());
#else              // Rest OS - Narrow!
    std::string& language = language_str_narrow;
#endif

    if (AK::StreamMgr::SetCurrentLanguage(language.data()) != AK_Success)
    {
        assert(!"Could not set language");
    }

    if (!SoundbankLoad(config.GetString("Defaults", "InitBank", "")))
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
    AK::SoundEngine::Term();
    AK::MemoryMgr::Term();

    std::cout << "Audio Engine Terminated" << std::endl;
}

std::string_view AudioEngine::GetSoundBanksBasePath()
{
    if (!IsInitialized()) { return {}; }
    return Get().soundbankBasePath;
}

std::string_view AudioEngine::GetExternalSourcesSubFolder()
{
    if (!IsInitialized()) { return {}; }
    return Get().externalSourcesSubFolder;
}

std::string_view AudioEngine::GetExternalSourcesBasePath()
{
    if (!IsInitialized()) { return {}; }
    return Get().externalSourcesBasePath;
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
    const AudioCallbackType callbackType, const AudioEventCallbackFunc callback, void* callbackCookie,
    const std::vector<AudioExternalSourceInfo>& ExternalSources)
{
    const AkGameObjectID ID = audioObjectID <= 0 || audioObjectID == AK_INVALID_GAME_OBJECT ? Get().mDefaultAudioObject : audioObjectID;

    AudioPosition position;
    position.Set({0,0,0},{1,0,0},{0,1,0});
    return PlayAudioEvent(eventName, position, ID, callbackType, callback, callbackCookie, ExternalSources);
}

uint32_t AudioEngine::PlayAudioEvent(const std::string& eventName, const AudioPosition& position,
    const uint64_t audioObjectID, const AudioCallbackType callbackType,
    const AudioEventCallbackFunc callback, void* callbackCookie, std::vector<AudioExternalSourceInfo> ExternalSources)
{
    if (!IsInitialized()) { return 0; }
    AK::SoundEngine::SetPosition(audioObjectID, position);

    return AK::SoundEngine::PostEvent(eventName.c_str(), audioObjectID,
        callbackType, callback, callbackCookie, ExternalSources.size(), ExternalSources.data());
}
void AudioEngine::StopPlayingAudioInstance(const uint32_t eventInstanceID, const int32_t transitionDurationMs, const AudioCurveInterpolation curve)
{
    if (!IsInitialized()) { return; }
    if (eventInstanceID == AK_INVALID_PLAYING_ID) { return; }
    AK::SoundEngine::ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Stop, eventInstanceID, transitionDurationMs, curve);
}

void AudioEngine::CancelAllCallbacksForAudioInstance(const uint32_t eventInstanceID)
{
    if (!IsInitialized()) { return; }
    if (eventInstanceID == AK_INVALID_PLAYING_ID) { return; }
    AK::SoundEngine::CancelEventCallback(eventInstanceID);
}

void AudioEngine::CancelAllCallbacksForAudioObject(const uint64_t audioObjectID)
{
    if (!IsInitialized()) { return; }
    if (audioObjectID == AK_INVALID_AUDIO_OBJECT_ID) { return; }
    AK::SoundEngine::CancelEventCallbackGameObject(audioObjectID);
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

uint32_t AudioEngine::GetAudioIDFromName(const std::string &name)
{
    if (!IsInitialized()) { return 0; }
    return AK::SoundEngine::GetIDFromString(name.c_str());
}

// Sound Engine Advanced

uint32_t AudioEngine::GetDeviceSampleRate()
{
    if (!IsInitialized()) { return 0; }
    return AK::SoundEngine::GetSampleRate();
}

bool AudioEngine::GetDeviceChannelConfigType(std::string& outChannelConfigType, uint32_t& outNumberOfChannels)
{
    if (!IsInitialized()) { return false; }
    outChannelConfigType = "None";
    outNumberOfChannels = 0;

    const std::unordered_map<AkUInt32, std::string> channelConfigurations {
        {AK_ChannelConfigType_Anonymous, "Anonymous"},
        {AK_ChannelConfigType_Standard, "Standard"},
        {AK_ChannelConfigType_Ambisonic, "Ambisonic"},
        {AK_ChannelConfigType_Objects, "Objects"}
    };

    AkChannelConfig channelConfig;
    Ak3DAudioSinkCapabilities audio3dCapabilities;

    if (const AkOutputDeviceID deviceId = AK::SoundEngine::GetOutputID(AK_INVALID_UNIQUE_ID, 0);
        AK::SoundEngine::GetOutputDeviceConfiguration(deviceId, channelConfig, audio3dCapabilities) != AK_Success)
    {
        return false;
    }

    outChannelConfigType = channelConfigurations.find(channelConfig.eConfigType)->second;
    outNumberOfChannels = channelConfig.uNumChannels;
    return true;
}

bool AudioEngine::GetDefaultAudioDeviceName(std::wstring& outCurrentDeviceName)
{
    if (!IsInitialized()) { return false; }

    constexpr AkUInt32 MaxNumberOfDevices = 50;

    const AkOutputDeviceID deviceId = AK::SoundEngine::GetOutputID(AK_INVALID_UNIQUE_ID, 0);
    AkUInt32 currentNumberOfDevices = MaxNumberOfDevices;
    AkDeviceDescription descriptions[MaxNumberOfDevices] = {};

    AK::SoundEngine::GetDeviceList(deviceId, currentNumberOfDevices, descriptions);
    for (size_t i = 0; i < currentNumberOfDevices; ++i)
    {
        const AkDeviceDescription& currentDevice = descriptions[i];
        if (currentDevice.isDefaultDevice)
        {
            // TODO: These character encoding conversions might not be the most robust. Please fix in the future!
#ifdef AK_OS_WCHAR // Windows only!
            outCurrentDeviceName.assign(currentDevice.deviceName);
#else
            std::string temp(currentDevice.deviceName);
            outCurrentDeviceName.assign(temp.begin(), temp.end());
#endif
            return true;
        }
    }
    return false;
}
