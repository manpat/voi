#include "apptime.h"

f64 AppTime::deltaTime = 0.0;
f64 AppTime::appTime = 0.0;
f64 AppTime::scaledAppTime = 0.0;
f64 AppTime::scaledDeltaTime = 0.0;
f64 AppTime::timescale = 1.0;
f64 AppTime::phystimescale = 1.0;

void AppTime::Update(f64 dt){
	deltaTime = dt;
	appTime += dt;

	scaledDeltaTime = dt * timescale;
	scaledAppTime += scaledDeltaTime;
}