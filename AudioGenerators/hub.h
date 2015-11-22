#ifndef AG_HUB_H
#define AG_HUB_H

#include "../audiogenerator.h"

struct HubAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		const f64 f = ntof(120-12);
		f64 ph = f * elapsed;

		f32 env = Env::Ramp((f32)elapsed, 10.f);

		auto o = Wave::Sin(ph);
		o += Wave::Sin(ph * (1.005f + Wave::Sin(elapsed * 0.1f) * 0.0002f)) * 0.3f;
		o += Wave::Sin(ph * 3.0f/2.0f) * 0.2f;

		o += (Wave::Saw(ph)*0.1f + Wave::Noise())
			* 0.08f * Wave::Sin(std::max((f32)(elapsed - 3.0) * 0.03f, 0.0f));

		return (f32)(o * env * 0.005f);
	}
};

#endif