#ifndef AG_AMBIENCE_H
#define AG_AMBIENCE_H

#include "audiogenerator.h"

struct LowRumbleAudioGenerator : AudioGenerator {
	f32 lowpass = 0.f;
	f32 o = 0.f;

	f32 Generate(f64 elapsed) override {
		constexpr f64 cutoff = 80.0;
		constexpr f64 RC = 1.0/(cutoff * 2.0*PI);
		constexpr f64 dt = 1.0/48000.0; // Assumption
		constexpr f64 a = dt/(RC + dt);

		o += Wave::Noise() * 0.05f;
		o = clamp(o, -1.0f, 1.0f);

		lowpass = lowpass + a * (o - lowpass);
		return lowpass * 0.1f * clamp(std::pow(elapsed, 3.f), 0.f, 1.f);
	}
};

#endif