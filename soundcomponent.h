#ifndef SOUNDCOMPONENT_H
#define SOUNDCOMPONENT_H

#include <fmod.hpp>

#include "component.h"
#include "common.h"

struct SoundComponent : Component {
	FMOD::Channel* channel = nullptr;
	FMOD::Sound* sound = nullptr;

	std::string fileName;
	f32 size;

	SoundComponent(const std::string&, f32 = 1.0f);
	void OnInit() override;
	void OnAwake() override;
	void OnUpdate() override;
	void OnDestroy() override;
};



#endif