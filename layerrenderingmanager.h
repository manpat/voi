#ifndef LAYER_RENDERING_MANAGER
#define LAYER_RENDERING_MANAGER

#include <OGRE/OgreRenderQueueListener.h>

#include "common.h"

enum {
	RENDER_QUEUE_LAYER = Ogre::RENDER_QUEUE_MAIN, // 5
	RENDER_QUEUE_PORTAL = Ogre::RENDER_QUEUE_2,
	RENDER_QUEUE_MIRRORED = Ogre::RENDER_QUEUE_6,
	RENDER_QUEUE_PARTICLES = Ogre::RENDER_QUEUE_9,
};

namespace Ogre {
	class RenderQueueInvocationSequence;
}

struct Camera;

struct LayerRenderingManager : Ogre::RenderQueueListener {
	LayerRenderingManager();
	~LayerRenderingManager();

	void SetupRenderQueueInvocationSequence(s32 layer);
	u32 GetNumLayers() const {return numLayers;}
	void SetCamera(Camera*);

	void renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) override;
	void renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) override;

	Ogre::RenderQueueInvocationSequence* rqis;
	Camera* camera;
	u32 numLayers = 0;
	u32 currentLayer;
};

#endif // LAYER_RENDERING_MANAGER