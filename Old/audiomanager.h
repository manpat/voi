#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <fmod.hpp>
#include <fmod_errors.h>

#include "audiogenerator.h"
#include "singleton.h"
#include "component.h"
#include "common.h"

struct AudioGeneratorFactoryBase;
struct AudioGenerator;

void cfmod(FMOD_RESULT result);
FMOD_VECTOR o2fm(const vec3&);

struct AudioManager : Singleton<AudioManager> {
	FMOD::System* system;
	FMOD::ChannelGroup* mastergroup;
	FMOD::DSP* lowPass;
	FMOD::DSP* reverb;
	f32 targetLowPassAmt = 22000.f;
	f32 targetReverbTime = 0.f;
	f32 targetReverbMix = 0.f;

	f32 lowPassAmt = 22000.f;
	f32 reverbTime = 0.f;
	f32 reverbMix = 0.f;

	std::map<std::string, std::shared_ptr<AudioGeneratorFactoryBase>> audioGeneratorTemplates;

	AudioManager();
	~AudioManager();

	void Update();
	void SetLowpass(f32);
	void SetReverbTime(f32);
	void SetReverbMix(f32);

	std::shared_ptr<AudioGenerator> CreateAudioGenerator(const std::string&);

	template<class GenType>
	void RegisterAudioGeneratorType(const std::string&);
};

struct AudioListenerComponent : Component {
	AudioListenerComponent();

	void OnUpdate() override;
};

#include "audiomanager.inl"

#endif