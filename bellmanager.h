#ifndef BELLMANAGER_H
#define BELLMANAGER_H

#include "audiogenerator.h"
#include "singleton.h"
#include "common.h"
#include <vector>

struct Bell;

struct BellManager : Singleton<BellManager> {
	std::vector<Bell*> bells;

	void AddBell(Bell*);
	void StopAllBells();
	void CorrectCombination();

	static void RegisterAudio();
};

struct BellAudioGenerator : AudioGenerator {
	u32 note = 120;
	f64 savedElapsed = -10000.0;
	bool triggered = false;
	bool playing = false;

	void Trigger() {
		triggered = true;
		playing = true;
	}

	void Stop() {
		playing = false;
	}

	f32 Generate(f64 elapsed) override {
		if(triggered){
			savedElapsed = elapsed;
			triggered = false;
		}

		if(playing) savedElapsed = elapsed;

		f32 env = 1.0 - clamp((elapsed-savedElapsed)/3.0, 0.0, 1.0);
		env = std::pow(env, 2.0);
		f64 f = ntof(note);
		f64 ph = f * elapsed;

		auto o = Wave::Sin(ph*0.5) * 0.05;
		o += Wave::Sin(ph) * env * 0.7;
		o += Wave::Triangle(ph*0.5) * (playing?0.5:0.0);

		return o;
	}
};

#endif