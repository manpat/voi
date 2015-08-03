#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <OGRE/OgreRenderQueueListener.h>
#include <OGRE/OgrePlane.h>
#include <string>
#include <vector>

#include "common.h"

enum {
	RENDER_QUEUE_PORTAL = Ogre::RENDER_QUEUE_3,
	RENDER_QUEUE_PORTALFRAME = Ogre::RENDER_QUEUE_4,
	RENDER_QUEUE_PORTALSCENE = Ogre::RENDER_QUEUE_MAIN,
};

namespace Ogre {
	class Root;
	class Viewport;
	class SubEntity;
	class RenderQueueInvocationSequence;
};

class Camera;

struct Portal {
	int id;
	int layer[2];

	Ogre::Plane clip;
};

class PortalManager : public Ogre::RenderQueueListener {
protected:
	Ogre::RenderQueueInvocationSequence* rqis;
	std::shared_ptr<Camera> camera;
	std::vector<Portal> portals;
	unsigned int numLayers;

public:
	PortalManager(Ogre::Root*, std::shared_ptr<Camera>&);

	void SetLayer(int);
	int AddPortal(Ogre::SubEntity*, int l0, int l1);

	Ogre::Plane portalClip;

protected:
	void renderQueueStarted(uint8_t queueId, const std::string& invocation, bool& skipThisInvocation) override;
	void renderQueueEnded(uint8_t queueId, const std::string& invocation, bool& repeatThisInvocation) override;
};

#endif