#ifndef AG_AMBIENCE_H
#define AG_AMBIENCE_H

#include "../audiogenerator.h"

struct LowRumbleAudioGenerator : AudioGenerator {
	f32 lowpass = 0.f;
	f32 o = 0.f;

	f32 Generate(f64 elapsed) override {
		const f64 cutoff = 50.0;
		const f64 RC = 1.0/(cutoff * 2.0*PI);
		const f64 dt = 1.0/48000.0; // Assumption
		const f64 a = dt/(RC + dt);

		o += Wave::Noise() * 0.05f;
		o = clamp(o, -1.0f, 1.0f);

		lowpass = lowpass + (f32)a * (o - lowpass);
		return lowpass * 0.05f * Env::ExpRamp((f32)elapsed, 1.f, 3.f);
	}
};

struct ChoirAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		(void) elapsed;

		return 0.f;
	}
};

#endif