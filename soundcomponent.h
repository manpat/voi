#ifndef SOUNDCOMPONENT_H
#define SOUNDCOMPONENT_H

#include <fmod.hpp>

#include "component.h"
#include "common.h"

struct SoundComponent : Component {
	FMOD::Channel* channel = nullptr;
	FMOD::Sound* sound = nullptr;

	std::string fileName;

	SoundComponent(const std::string&);
	void OnInit() override;
	void OnUpdate() override;
	void OnDestroy() override;
};



#endif