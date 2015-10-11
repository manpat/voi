#ifndef AUDIOGENERATOR_H
#define AUDIOGENERATOR_H

#include "common.h"

namespace Wave {
	f32 Sin(f64 phase);
	f32 Triangle(f64 phase);
	f32 Square(f64 phase, f64 width = 0.5);
	f32 Saw(f64 phase);

	f32 Noise();
}

#endif