#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>
#include <AkFilePackageLowLevelIODeferred.h> // Low Level IO implementation in the "/samples/SoundEngine" folder

#ifndef AK_OPTIMIZED // The profiler module
    #include <AK/Comm/AkCommunication.h>
#endif

#include <memory>
#include <string>

using AudioPosition = AkSoundPosition;
using AudioCurveInterpolation = AkCurveInterpolation;
using AudioBankType = AkBankType;

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

		static bool LoadSoundBank(const std::string& bank, AudioBankType type = AkBankType_User);
		static bool UnloadSoundBank(const std::string& bank, AudioBankType type = AkBankType_User);

		// Listener

		static void SetDefaultListener(uint64_t audioObjectID);

		// Audio Objects (AkGameObjectID)

		static uint64_t GetNewAudioObjectID();
		static bool RegisterAudioObject(uint64_t audioObjectID, const std::string& name = std::string());
		static bool UnregisterAudioObject(uint64_t audioObjectID);
		static bool UnregisterAllAudioObjects();
		static bool AudioObjectSetPosition(uint64_t audioObjectID, const AudioPosition& position);

		// Events

		static uint32_t PlayAudioEvent(const std::string& eventName,
			uint64_t audioObjectID = AK_INVALID_GAME_OBJECT);
		static uint32_t PlayAudioEvent(const std::string& eventName,
			const AudioPosition& position,
			uint64_t audioObjectID);

		// Parameters

		static bool SetState(const std::string& stateGroup, const std::string& stateValue);
		static void SetSwitch(const std::string& switchGroup, const std::string& switchValue, uint64_t audioObjectID);
		static bool SetParameter(const std::string& parameterName, float value,
			uint64_t audioObjectID = AK_INVALID_GAME_OBJECT, int valueChangeDuration = 0,
			AudioCurveInterpolation curveInterpolation = AkCurveInterpolation_Linear,
			bool bBypassInternalInterpolation = false);

	private:

		static std::unique_ptr<AudioEngine> sInstance;
		static uint64_t nextAudioObjectID;
		AkGameObjectID defaultAudioObject;
		AkGameObjectID defaultAudioListener;

#pragma region SETTINGS

		AkMemSettings mMemorySettings;
		AkStreamMgrSettings mStreamingSettings;
		AkDeviceSettings mStreamingDeviceSettings;
		CAkFilePackageLowLevelIODeferred mLowLevelIO;
		AkInitSettings mInitSettings;
		AkPlatformInitSettings mPlatformInitSettings;
		AkMusicSettings mMusicSettings;
		AkSpatialAudioInitSettings mSpatialAudioSettings;

#ifndef AK_OPTIMIZED
		AkCommSettings mCommunicationSettings;
#endif

		AudioEngine();

		// Set Init settings after AK::SoundEngine::GetDefaultInitSettings
		void SetInitSettings();
		bool Initialize_Internal();

#pragma endregion
};
#endif
