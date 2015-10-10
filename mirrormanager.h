#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"

struct Mirror : Component {
	// TODO

	void OnInit() override;
	void OnDestroy() override;
};

class MirrorManager : public Ogre::RenderQueueListener {
	// TODO
};

#endif // MIRROR_MANAGER_H