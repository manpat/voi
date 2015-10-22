#ifndef BELL_H
#define BELL_H

#include "component.h"

struct Bell : Component {
	Entity* target = nullptr;
	std::string targetName;

	Bell(const std::string& tn) : Component(this), targetName(tn) {}

	static void RegisterAudio();
	void OnAwake() override;
};

#endif