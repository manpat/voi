#ifndef HUBMANAGER_H
#define HUBMANAGER_H

#include "singleton.h"
#include "common.h"

struct HubManager : Singleton<HubManager> {
	s32 lastLevelCompleted = -1;
	bool doneMyThang = true;

	void NotifyHubLoad();
	void NotifyReturnToHub();
	void Update();
};

#endif