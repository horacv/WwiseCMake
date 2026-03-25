#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

// Low Level IO implementation in the "/samples/SoundEngine" folder
#include <AkFilePackageLowLevelIODeferred.h>
// Query namespace to retrieve data from Wwise
#include "AK/SoundEngine/Common/AkQueryParameters.h"

#include <memory>
#include <string>

#include "audio_config.h"

struct AkSpatialAudioInitSettings;

using AudioBankType = AkBankType;
using AudioCallbackInfo = AkCallbackInfo;
using AudioCallbackType = AkCallbackType;
using AudioCurveInterpolation = AkCurveInterpolation;
using AudioCallbackFunc = AkCallbackFunc;
using AudioPosition = AkSoundPosition;
using AudioParameterType = AK::SoundEngine::Query::RTPCValue_type;
using AudioCurveInterpolation = AkCurveInterpolation;
using AudioExternalSourceInfo = AkExternalSourceInfo;

class AudioEngine
{
	public:
		static AudioEngine& Get();

		static bool Initialize();
		static void Terminate();

		static std::string_view GetSoundBanksBasePath();
		static std::string_view GetExternalSourcesSubFolder();
		static std::string_view GetExternalSourcesBasePath();

		// Call this every frame on the program's update loop
		static void Update();
		static bool IsInitialized();

		// Soundbanks

		static bool SoundbankLoad(const std::string& bank, AudioBankType type = AkBankType_User);
		static bool SoundbankUnload(const std::string& bank, AudioBankType type = AkBankType_User);

		// Listener

		static void SetDefaultListener(uint64_t audioObjectID);

		// Audio Objects

		static uint64_t AudioObjectGetNewID();
		static bool AudioObjectRegister(uint64_t audioObjectID, const std::string& name = std::string());
		static bool AudioObjectUnregister(uint64_t audioObjectID);
		static bool AudioObjectUnregisterAll();
		static bool AudioObjectSetPosition(uint64_t audioObjectID, const AudioPosition& position);

		// Events

		static uint32_t PlayAudioEvent(const std::string& eventName,
			uint64_t audioObjectID = AK_INVALID_GAME_OBJECT,
			AudioCallbackType callbackType = AK_CallbackBits,
			AudioCallbackFunc callback = nullptr,
			void* callbackCookie = nullptr,
			const std::vector<AudioExternalSourceInfo>& ExternalSources = {});

		static uint32_t PlayAudioEvent(const std::string& eventName,
			const AudioPosition& position,
			uint64_t audioObjectID,
			AudioCallbackType callbackType = AK_CallbackBits,
			AudioCallbackFunc callback = nullptr,
			void* callbackCookie = nullptr,
			std::vector<AudioExternalSourceInfo> ExternalSources = {});

		static void StopPlayingAudioInstance(uint32_t eventInstanceID,
			int32_t transitionDurationMs = 0, AudioCurveInterpolation curve = AkCurveInterpolation_Linear);

		static void CancelAllCallbacksForAudioInstance(uint32_t eventInstanceID);
		static void CancelAllCallbacksForAudioObject(uint64_t audioObjectID);

		// Parameters

		static bool SetState(const std::string& stateGroup, const std::string& stateValue);
		static bool SetSwitch(const std::string& switchGroup, const std::string& switchValue, uint64_t audioObjectID);

		static bool SetParameter(const std::string& parameterName, float value,
			uint64_t audioObjectID = AK_INVALID_GAME_OBJECT, int valueChangeDuration = 0,
			AudioCurveInterpolation curveInterpolation = AkCurveInterpolation_Linear,
			bool bBypassInternalInterpolation = false);

		static bool GetParameter(const std::string& parameterName, float& outValue,
			AudioParameterType inParameterType, AudioParameterType& outParameterType,
			uint64_t inAudioObjectID = AK_INVALID_GAME_OBJECT,uint32_t inEventInstanceID = -1);

		// Sound Engine Advanced

		static uint32_t GetAudioIDFromName(const std::string& name);
		static uint32_t GetDeviceSampleRate();
		static bool GetDeviceChannelConfigType(std::string& outChannelConfigType, uint32_t& outNumberOfChannels);
		static bool GetDefaultAudioDeviceName(std::wstring& outCurrentDeviceName);

	private:
		static std::unique_ptr<AudioEngine> sInstance;
		static uint64_t sNextAudioObjectID;
		AkGameObjectID mDefaultAudioObject;
		AkGameObjectID mDefaultAudioListener;
		CAkFilePackageLowLevelIODeferred mLowLevelIO;

		std::string soundbankBasePath;
		std::string externalSourcesSubFolder;
		std::string externalSourcesBasePath;

		AudioEngine();
};
#endif
