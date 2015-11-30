#ifndef AG_BELL_H 
#define AG_BELL_H 

#include "../audiogenerator.h"

struct BellAudioGenerator : AudioGenerator {
	u32 note = 120;
	f64 savedElapsed = -10000.0;
	bool triggered = false;
	bool playing = false;
	bool correct = false;

	void Start() override {
		if(correct) return;

		triggered = true;
		playing = true;
	}

	void Stop() override {
		playing = false;
	}

	void SetParam(u32 p, s64 val) {
		switch(p){
			case 0: // Set note
				note = val;
				break;

			case 1: // Combination
				if(val == 1) { // correct
					correct = true;
					playing = false;
				}
				break;
		}
	}

	f32 Generate(f64 elapsed) override {
		if(triggered){
			savedElapsed = elapsed;
			triggered = false;
		}

		if(playing) savedElapsed = elapsed;

		f64 env = 1.0 - Env::Ramp(elapsed-savedElapsed, 3.f);
		// env = std::pow(env, 2.f);

		f64 f = ntof(note + (correct?12:0));
		f64 ph = f * elapsed;

		f32 o = 0.f; 
		o += Wave::Triangle(ph) * env * 0.5f;
		o += Wave::Sin(ph) * (playing?1.0f:0.0f) * 0.1f;

		return o * Env::Ramp((f32)elapsed, 1.f);
	}
};

#endif