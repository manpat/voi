#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <OGRE/OgreRenderQueueListener.h>
#include <OGRE/OgrePlane.h>
#include <string>
#include <vector>

#include "common.h"
#include "component.h"

enum {
	RENDER_QUEUE_PORTAL = Ogre::RENDER_QUEUE_2,
	RENDER_QUEUE_PORTALSCENE = Ogre::RENDER_QUEUE_MAIN,
	RENDER_QUEUE_PARTICLES = Ogre::RENDER_QUEUE_9,
};

namespace Ogre {
	class Root;
	class Viewport;
	class SubEntity;
	class RenderQueueInvocationSequence;
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

class PortalManager : public Ogre::RenderQueueListener {
protected:
	Ogre::RenderQueueInvocationSequence* rqis;
	Camera* camera;
	std::vector<Portal*> portals;
	u32 numLayers;
	u32 currentLayer;

public:
	PortalManager();
	~PortalManager();

	void SetLayer(s32);
	u32 GetNumLayers() const {return numLayers;}

	void AddPortal(Portal*);
	void SetCamera(Camera*);

protected:
	void renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) override;
	void renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) override;
};

#endif