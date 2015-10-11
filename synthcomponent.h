#ifndef SYNTHCOMPONENT_H
#define SYNTHCOMPONENT_H

#include <fmod.hpp>

#include "component.h"
#include "common.h"

struct SynthComponent : Component {
	FMOD::Channel* channel = nullptr;
	FMOD::DSP* dsp = nullptr;

	f64 elapsed = 0.0;

	// This is most definitely temporary
	u32 mode = 0;

	SynthComponent(u32 = 0);
	void OnInit() override;
	void OnUpdate() override;
	void OnDestroy() override;

	f32 Generate(f64);

private:
	static FMOD_RESULT F_CALLBACK GeneratorFunction(FMOD_DSP_STATE*, f32*, f32*, u32, s32, s32*);
};

#endif