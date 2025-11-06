#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

// Low Level IO implementation in the "/samples/SoundEngine" folder
#include <AkFilePackageLowLevelIODeferred.h>

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

class AudioEngine
{
	public:
		static AudioEngine& Get();

		static bool Initialize();
		static void Terminate();

		// Call this every frame on the program's update loop
		static void RenderAudio();
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
			void* callbackCookie = nullptr);

		static uint32_t PlayAudioEvent(const std::string& eventName,
			const AudioPosition& position,
			uint64_t audioObjectID,
			AudioCallbackType callbackType = AK_CallbackBits,
			AudioCallbackFunc callback = nullptr,
			void* callbackCookie = nullptr);

		// Parameters

		static bool SetState(const std::string& stateGroup, const std::string& stateValue);
		static bool SetSwitch(const std::string& switchGroup, const std::string& switchValue, uint64_t audioObjectID);
		static bool SetParameter(const std::string& parameterName, float value,
			uint64_t audioObjectID = AK_INVALID_GAME_OBJECT, int valueChangeDuration = 0,
			AudioCurveInterpolation curveInterpolation = AkCurveInterpolation_Linear,
			bool bBypassInternalInterpolation = false);

	private:
		static std::unique_ptr<AudioEngine> sInstance;
		static uint64_t sNextAudioObjectID;
		AkGameObjectID mDefaultAudioObject;
		AkGameObjectID mDefaultAudioListener;
		CAkFilePackageLowLevelIODeferred mLowLevelIO;

		AudioEngine();
};
#endif
