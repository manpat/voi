#ifndef HALFLIFEPOINTMANAGER_H
#define HALFLIFEPOINTMANAGER_H

#include "singleton.h"

struct HalfLifePointComponent;

struct HalfLifePointManager : Singleton<HalfLifePointManager> {
	std::string toLevel;
	std::string toNode;
	quat rotOffset;
	vec3 posOffset;

	HalfLifePointManager();

	void TriggerSceneLoad(HalfLifePointComponent*, vec3, quat);
	void Update();
};

#endif