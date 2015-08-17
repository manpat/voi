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
	RENDER_QUEUE_PARTICLES = Ogre::RENDER_QUEUE_9,
};

namespace Ogre {
	class Root;
	class Viewport;
	class SubEntity;
	class RenderQueueInvocationSequence;
};

class Camera;

struct Portal {
	s32 id;
	s32 layer[2];

	Ogre::Plane clip;
};

class PortalManager : public Ogre::RenderQueueListener {
protected:
	Ogre::RenderQueueInvocationSequence* rqis;
	std::shared_ptr<Camera> camera;
	std::vector<Portal> portals;
	u32 numLayers;
	u32 currentLayer;

public:
	PortalManager(Ogre::Root*, std::shared_ptr<Camera>&);

	void SetLayer(s32);
	u32 GetNumLayers() const {return numLayers;}

	// Note: AddPortal assumes that the portal submesh's normal lies along the Z-axis
	//	local to the parent mesh
	void AddPortal(Ogre::Entity*, s32 l0, s32 l1);

protected:
	void renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) override;
	void renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) override;
};

#endif