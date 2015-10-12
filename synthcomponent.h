#ifndef SYNTHCOMPONENT_H
#define SYNTHCOMPONENT_H

#include <fmod.hpp>

#include "component.h"
#include "common.h"

struct AudioGenerator;

struct SynthComponent : Component {
	FMOD::Channel* channel = nullptr;
	FMOD::DSP* dsp = nullptr;
	std::shared_ptr<AudioGenerator> generator;

	f64 elapsed = 0.0;

	std::string synthName;
	f32 size;

	SynthComponent(const std::string&, f32 = 1.0f);
	void OnInit() override;
	void OnAwake() override;
	void OnUpdate() override;
	void OnDestroy() override;

	f32 Generate(f64);

private:
	static FMOD_RESULT F_CALLBACK GeneratorFunction(FMOD_DSP_STATE*, f32*, f32*, u32, s32, s32*);
};

#endif