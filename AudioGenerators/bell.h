#ifndef AG_BELL_H 
#define AG_BELL_H 

#include "../audiogenerator.h"

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

		f64 env = 1.0 - clamp((elapsed-savedElapsed)/2.0, 0.0, 1.0);
		env = std::pow(env, 2.0);

		f64 f = ntof(note);
		f64 ph = f * elapsed;

		auto o = Wave::Sin(ph*0.5) * 0.05;
		o += Wave::Sin(ph) * env * 0.3;
		o += Wave::Triangle(ph*0.5) * (playing?0.3:0.0);

		return (f32)o * Env::Ramp((f32)elapsed, 3.f) * 0.5f;
	}
};

#endif