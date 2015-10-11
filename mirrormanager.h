#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"

struct Mirror : Component {
	s32 layer[2];
	// TODO

	Mirror(s32 l0, s32 l1);

	void OnInit() override;
	void OnDestroy() override;
};

class MirrorManager : public Ogre::RenderQueueListener {
	// TODO
};

#endif // MIRROR_MANAGER_H