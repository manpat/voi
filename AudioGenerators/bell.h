#ifndef AG_BELL_H 
#define AG_BELL_H 

#include "../audiogenerator.h"

struct BellAudioGenerator : AudioGenerator {
	u32 note = 120;
	f64 savedElapsed = -10000.0;
	bool triggered = false;
	bool playing = false;
	u8 correctness = 0; // 0 - none, 1 - correct, 2 - incorrect

	void Start() override {
		if(correctness == 1) return;

		correctness = 0;
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
				correctness = val;
				playing = false;
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

		const f64 incorrectRatio = 16.5f / 17.f;
		f64 f = ntof(note + (correctness == 1 ? 12 : 0)) 
			* ((correctness == 2) ? incorrectRatio : 1.0);

		f64 ph = f * elapsed;

		f32 o = 0.f; 
		o += Wave::Sin(ntof(120) * 0.5 * elapsed) * ((correctness == 1) ? 0.f : 0.02f);
		o += Wave::Triangle(ph) * env * 0.5f;
		o += Wave::Sin(ph) * (playing?1.0f:0.0f) * 0.1f;

		return o * Env::Ramp((f32)elapsed, 1.f);
	}
};

#endif