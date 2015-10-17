#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"
#include "singleton.h"

namespace Ogre {
	class Camera;
}

struct Mirror : Component {
	Mirror(s32 l0, s32 l1);

	void OnInit() override;
	void OnUpdate() override;

	s32 mirrorId;
	s32 layer[2]; // Layers that should be reflected (layer of mirror, normally hidden layer)
	bool isVisible = true;
	u8 maxBounces = 2; // Maximum number of times a mirror should reflect per frame (recursivley ie. two opposing mirrors)
	u8 bounceCount = 0;
};

struct MirrorManager : public Singleton<MirrorManager>, Ogre::RenderQueueListener {
public:
	MirrorManager();
	~MirrorManager();

	void AddMirror(Mirror*);

	std::vector<Mirror*> mirrors;
};

#endif // MIRROR_MANAGER_H