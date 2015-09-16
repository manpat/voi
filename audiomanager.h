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
	FmodSystemRef(FMOD::System* s) {}

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

	void update(u64);
	void playNote(u8);
	FmodSystemRef getSystem();
	FMOD::ChannelGroup* getMasterChannelGroup() { return mastergroup; }
	bool isRunning() const { return running; };

	u8 BPM;

private:
	FMOD::System* system;
	FMOD::ChannelGroup* mastergroup;
	AudioSynth* synth;

	bool running = false;
};

#endif //AUDIOMANAGER_H