#ifndef AG_HUB_H
#define AG_HUB_H

#include "audiogenerator.h"

struct HubAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		f64 f = ntof(120-12);
		f64 ph = f * elapsed;

		f64 env = std::min(elapsed * 0.1, 1.0);

		auto o = Wave::Sin(ph);
		o += Wave::Sin(ph * (1.005 + Wave::Sin(elapsed * 0.1) * 0.0002)) * 0.3;
		o += Wave::Sin(ph * 3.0/2.0) * 0.2;

		// o += Wave::Noise() * 0.15 * Wave::Sin(std::max((elapsed - 3.0) * 0.03, 0.0));
		o += (Wave::Saw(ph)*0.1 + Wave::Noise()) * 0.08 * Wave::Sin(std::max((elapsed - 3.0) * 0.03, 0.0));

		return (f32)(o * env * 0.05);
	}
};

#endif