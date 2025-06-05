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

class AudioEngine
{
	public:
		static AudioEngine& Get();

		bool Initialize();
		void Terminate();

		// Call this every frame on the program's update loop
		static void RenderAudio();

		static bool IsInitialized();
		static void SetDefaultListener(uint64_t audioObjectID);
		static bool LoadSoundBank(const std::string& bank);
		static uint64_t GetNewAudioObjectID();
		static bool RegisterAudioObject(uint64_t audioObjectID, const std::string& name);
		static uint32_t PlayAudioEvent(const std::string& eventName, uint64_t audioObjectID = 0);

	private:

		static std::unique_ptr<AudioEngine> sInstance;

		static uint64_t nextAudioObjectID;
		AkGameObjectID defaultAudioObject;
		AkGameObjectID defaultAudioListener;

		AudioEngine();

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

		// Set Init settings after AK::SoundEngine::GetDefaultInitSettings
		void SetInitSettings();

#pragma endregion
};
#endif
