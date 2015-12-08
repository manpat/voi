#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <string>
#include <vector>
#include <OGRE/OgreSharedPtr.h>

#include "common.h"
#include "singleton.h"
#include "component.h"

namespace Ogre {
	class Root;
	class Viewport;
	class Material;
	class SubEntity;
	class ColourValue;

	using MaterialPtr = SharedPtr<Material>;
}

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
	Ogre::MaterialPtr portalMaterial;

	PortalManager();
	~PortalManager();

	void AddPortal(Portal*);
	void SetPortalColor(const Ogre::ColourValue&);
};

#endif