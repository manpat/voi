#ifndef AG_BELL_H 
#define AG_BELL_H 

#include "../audiogenerator.h"

struct BellAudioGenerator : AudioGenerator {
	u32 note = 120;
	f64 sustainedElapsed = -10000.0;
	f64 triggerElapsed = -10000.0;
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
				note = (u32)val;
				break;

			case 1: // Combination
				correctness = (u8)val;
				playing = false;
				break;
		}
	}

	f32 Generate(f64 elapsed) override {
		if(triggered){
			sustainedElapsed = triggerElapsed = elapsed;
			triggered = false;
		}

		if(playing) sustainedElapsed = elapsed;

		f32 env = 1.f - Env::Ramp((f32)(elapsed - triggerElapsed), 1.f);
		f32 env2 = 1.f - Env::Ramp((f32)(elapsed - sustainedElapsed), 3.f);
		env2 = std::pow(env2, 2.f);

		const f64 incorrectRatio = 16.5f / 17.f;
		f64 f = ntof(note) 
			* ((correctness == 2) ? incorrectRatio : 1.0);

		f64 ph = f * elapsed;
		f64 rootPhase = ntof(120 - 12) * elapsed;

		f32 o = 0.f; 
		o += Wave::Sin(rootPhase) * ((correctness == 1) ? 0.f : 0.02f);

		o += Wave::Sin(ph) * env * 0.5f;
		o += Wave::Sin(ph * 2.f) * env * 0.25f;

		f32 harmonic = 0.f;
		harmonic += Wave::Sin(ph * 0.5f) / 1.f;
		harmonic += Wave::Sin(ph * 1.0f) / 2.f;
		harmonic += Wave::Sin(ph * 2.0f) / 3.f;

		if(correctness == 1) {
			harmonic += Wave::Triangle(ph * 2.0f);
		}

		o += harmonic * env2 * 0.25f;

		f32 fadeInFromStart = Env::Ramp((f32)elapsed, 1.f);
		return o * fadeInFromStart * 0.5f;
	}
};

#endif