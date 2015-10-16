#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

//#include "common.h"
#include "component.h"

namespace Ogre {
	class Camera;
}

struct Mirror : Component {
	Mirror(s32 l0, s32 l1);

	s32 layer[2]; // Layers that should be reflected (layer of mirror, normally hidden layer)
	Ogre::Camera* ogreCamera;
};

class MirrorManager : public Ogre::RenderQueueListener {
public:
	MirrorManager();
	~MirrorManager();

	void AddMirror(Mirror*);

protected:
	void renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) override;
	void renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) override;

	std::vector<Mirror*> mirrors;

private:

};

#endif // MIRROR_MANAGER_H