#ifndef AG_SFX_H
#define AG_SFX_H

#include "../audiogenerator.h"

struct DoorGrindAudioGenerator : AudioGenerator {
	f32 lowpass = 0.f;
	f32 o = 0.f;
	bool paused = true;

	void Start() override { paused = false; }
	void Stop() override { paused = true; }

	f32 Generate(f64 elapsed) override {
		if(paused) return 0.f;

		const f64 cutoff = 200.0;
		const f64 RC = 1.0/(cutoff * 2.0*PI);
		const f64 dt = 1.0/48000.0; // Assumption
		const f64 a = dt/(RC + dt);
		const f32 dirtySteps = 4.f;

		o += Wave::Noise() * 0.5f;
		o = floor(o*dirtySteps)/dirtySteps;
		o = clamp(o, -1.0f, 1.0f);

		lowpass = lowpass + (f32)a * (o - lowpass);
		return lowpass * Env::ExpRamp((f32)elapsed, 1.f, 3.f) * 0.6f;
	}
};

#endif