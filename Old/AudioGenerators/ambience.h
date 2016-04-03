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

static f32 AhFormant(f64 elapsed, f64 freq) {
	// Formant frequencies: [750.0 940.0]
	f32 o = 0.f;

	o += Wave::Sin(750.0 * elapsed);
	o += Wave::Sin(940.0 * elapsed);
	o += Wave::Noise() * 0.2f;

	return o * 0.5f;
}

struct ChoirAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		f32 o = 0.f;

		o += AhFormant(elapsed, 220.0);
		// o += Wave::Sin(elapsed * 220.0);
		// o += Wave::Sin(elapsed * 220.0 * 3.0/2.0) / 2.f;
		// o += Wave::Triangle(elapsed * 440.0) / 2.f;
		// o += Wave::Triangle(elapsed * 880.0) / 3.f;
		// o += Wave::Triangle(elapsed * 1760.0) / 4.f;
		// o += Wave::Noise() * 0.03f;

		return o * Env::ExpRamp(elapsed, 10.f) * 0.5f;
	}
};

#endif