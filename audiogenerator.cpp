#include "audiogenerator.h"

#include <random>
#include <cmath>

namespace Wave {
	// TODO: These can all be put into buffers

	f32 Sin(f64 phase){
		return (f32)std::sin(2.0*M_PI*phase);
	}

	f32 Triangle(f64 phase){
		auto nph = std::fmod(phase, 1.0);
		if(nph <= 0.5) return (f32)((nph-0.25)*4.0);

		return (f32)((0.75-nph)*4.0);
	}

	f32 Square(f64 phase, f64 width){
		auto nph = std::fmod(phase, 1.0);
		if(nph < width) return -1.0;

		return 1.0;
	}

	f32 Saw(f64 phase){
		return (f32)(std::fmod(phase*2.0, 2.0)-1.0);
	}

	f32 Noise(){
		static std::default_random_engine generator;
		static std::uniform_real_distribution<f32> distribution(-1.0f, 1.0f);

		return distribution(generator);
	}
}