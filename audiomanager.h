#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <fmod.hpp>
#include <fmod_errors.h>

#include "singleton.h"
#include "component.h"
#include "common.h"

void cfmod(FMOD_RESULT result);
FMOD_VECTOR o2fm(const vec3&);

struct AudioManager : Singleton<AudioManager> {
	FMOD::System* system;
	FMOD::ChannelGroup* mastergroup;

	AudioManager();
	~AudioManager();

	void Update();
};

struct AudioListenerComponent : Component {
	AudioListenerComponent();

	void OnUpdate() override;
};

#endif