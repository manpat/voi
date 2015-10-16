#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <string>
#include <vector>

#include "common.h"
#include "singleton.h"
#include "component.h"

namespace Ogre {
	class Root;
	class Viewport;
	class SubEntity;
};

struct Camera;

struct Portal : Component {
	s32 portalId;
	s32 layer[2];
	Ogre::Plane clip;
	bool shouldDraw;

	Portal(s32 l0, s32 l1);

	void OnInit() override;
};

struct PortalManager : Singleton<PortalManager> {
	std::vector<Portal*> portals;

	PortalManager();
	~PortalManager();

	void AddPortal(Portal*);
};

#endif