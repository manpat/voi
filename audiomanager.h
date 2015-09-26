#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <iostream>

#include <fmod.hpp>
#include <fmod_errors.h>

#include "audiosynth.h"
#include "common.h"

void cfmod(FMOD_RESULT result);

class FmodSystemRef {
	FMOD::System* system;

public:
	FmodSystemRef(FMOD::System* s) : system{s} {}

	FMOD::System* operator->() {
		return system;
	}
	FMOD::System& operator*() {
		return *system;
	}
};

class AudioManager {
public:
	AudioManager();
	~AudioManager();

	void Update();
	void PlayNote(u8);
	FmodSystemRef GetSystem();
	FMOD::ChannelGroup* GetMasterChannelGroup() { return mastergroup; }
	bool IsRunning() const { return running; };

	u8 BPM;

private:
	FMOD::System* system;
	FMOD::ChannelGroup* mastergroup;
	AudioSynth* synth;

	bool running = false;
};

#endif //AUDIOMANAGER_H