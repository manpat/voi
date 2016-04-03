#include "audiogenerator.h"

#include <random>
#include <cmath>
#include <array>

enum { WaveSamples = 44100 };
f32 noiseTable[WaveSamples * 4];
f32 sinTable[WaveSamples];
f32 triTable[WaveSamples];
f64 freqTable[256];

namespace Wave {

	f32 Sin(f64 phase){
		// return (f32)std::sin(2.0*M_PI*phase);

		return sinTable[(u64)(phase * WaveSamples) % WaveSamples];
	}

	f32 Triangle(f64 phase){
		// auto nph = std::fmod(phase, 1.0);
		// if(nph <= 0.5) return (f32)((nph-0.25)*4.0);

		// return (f32)((0.75-nph)*4.0);
		return triTable[(u64)(phase * WaveSamples) % WaveSamples];
	}

	f32 Square(f64 phase, f64 width){
		// phase -= std::floor(phase);
		phase = std::fmod(phase, 1.0);
		if(phase < width) return -1.0;

		return 1.0;
	}

	f32 Saw(f64 phase){
		return (f32)(std::fmod(phase*2.0, 2.0)-1.0);
	}

	f32 Noise(){
		static u32 i = 0;
		return noiseTable[(i++)%WaveSamples];
	}

	void InitTables() {
		std::default_random_engine generator;
		std::uniform_real_distribution<f32> distribution(-1.0f, 1.0f);

		for(u32 i = 0; i < WaveSamples; i++) {
			sinTable[i] = (f32)std::sin(2.0 * PI * i / (f64)WaveSamples);
		}

		for(u32 i = 0; i < WaveSamples; i++) {
			f32 val = 0.f;

			auto nph = std::fmod(i / (f64)WaveSamples, 1.0);
			if(nph <= 0.5) val = (f32)((nph-0.25)*4.0);
			else val = (f32)((0.75-nph)*4.0);

			triTable[i] = val;
		}

		for(u32 i = 0; i < WaveSamples; i++) {
			noiseTable[i] = distribution(generator);
		}

		for(s32 i = 0; i < 256; i++) {
			auto freq = 220.0 * std::pow(2.0, (i - 128) / 12.0);
			freqTable[i] = freq;
		}
	}
}

namespace Env {
	f32 Ramp(f32 phase, f32 length) {
		return clamp(phase/length, 0.f, 1.f);
	}

	f32 ExpRamp(f32 phase, f32 length, f32 exp) {
		return clamp(std::pow(phase/length, exp), 0.f, 1.f);
	}
}

f64 ntof(u8 n) {
	return freqTable[n];
}