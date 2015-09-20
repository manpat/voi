#ifndef APPTIME_H
#define APPTIME_H

#include "common.h"

struct AppTime {
	static f64 appTime;
	static f64 deltaTime;
	static f64 scaledAppTime;
	static f64 scaledDeltaTime;
	static f64 timescale;

	static void Update(f64);
};

#endif