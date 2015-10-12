#ifndef AUDIOGENERATOR_H
#define AUDIOGENERATOR_H

#include "common.h"

f64 ntof(u8 n);

namespace Wave {
	f32 Sin(f64 phase);
	f32 Triangle(f64 phase);
	f32 Square(f64 phase, f64 width = 0.5);
	f32 Saw(f64 phase);

	f32 Noise();
}

// Inherit from this
struct AudioGenerator {
	virtual ~AudioGenerator() {};
	virtual f32 Generate(f64 phase) = 0;
};

struct AudioGeneratorFactoryBase {
	virtual std::shared_ptr<AudioGenerator> Create() = 0;
};

template<class GenType>
struct AudioGeneratorFactory : AudioGeneratorFactoryBase {
	std::shared_ptr<AudioGenerator> Create() override { return std::make_shared<GenType>(); }
};

#endif