#include "controlmap.h"

#include "input.h"

ControlMap::ControlMap() {
	forward = SDLK_w;
	backward = SDLK_s;
	left = SDLK_a;
	right = SDLK_d;
	boost = SDLK_LSHIFT;
	jump = SDLK_SPACE;
	interact = SDLK_e;
}