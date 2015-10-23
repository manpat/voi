#ifndef AREATRIGGERMANAGER_H
#define AREATRIGGERMANAGER_H

#include "singleton.h"

struct AreaTriggerComponent;

struct AreaTriggerManager : Singleton<AreaTriggerManager> {
	std::string toLevel;
	std::string toNode;
	quat rotOffset;
	vec3 posOffset;

	AreaTriggerManager();

	void TriggerSceneLoad(AreaTriggerComponent*, vec3, quat);
	void Update();
};

#endif