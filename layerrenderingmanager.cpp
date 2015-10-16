#include <OGRE/OgreRenderQueueInvocation.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>

#include "layerrenderingmanager.h"
#include "portalmanager.h"
#include "camera.h"
#include "app.h"

#include <cassert>

LayerRenderingManager::LayerRenderingManager() {
	rqis = App::GetSingleton()->ogreRoot->createRenderQueueInvocationSequence("LayerInvocationSequence");
	App::GetSingleton()->sceneManager->addRenderQueueListener(this);

	numLayers = 1;
}

LayerRenderingManager::~LayerRenderingManager() {
	App::GetSingleton()->ogreRoot->destroyRenderQueueInvocationSequence("LayerInvocationSequence");
	App::GetSingleton()->sceneManager->removeRenderQueueListener(this);

	rqis = nullptr;
}

void LayerRenderingManager::SetCamera(Camera* c){
	camera = c;
	camera->viewport->setRenderQueueInvocationSequenceName("LayerInvocationSequence");
}

void LayerRenderingManager::SetupRenderQueueInvocationSequence(s32 l) {
	assert(l < (s32)numLayers);
	currentLayer = l;

	rqis->clear();
	rqis->add(RENDER_QUEUE_LAYER+l, "Main");

	using PType = std::pair<s32, Portal*>;
	static std::vector<PType> visiblePortals;
	visiblePortals.clear();

	for(auto p: PortalManager::GetSingleton()->portals){
		// Test if portal exists in the new layer and if so
		//	get it's destination layer
		s32 layer = -1;
		if(p->layer[0] == l){
			layer = p->layer[1];

		}else if(p->layer[1] == l){
			layer = p->layer[0];
		}

		// if the portal IS in the new layer
		if(layer >= 0){
			// Queue it's frame to be rendered
			// rqis->add(RENDER_QUEUE_PORTALFRAME+p->portalId, "Main");
			visiblePortals.push_back(PType(layer, p));
		}
	}

	// Prepare for portal drawing by clearing the stencil buffer
	rqis->add(1, "PrepPrt");

	// Draw each portal to the stencil buffer with a ref value of 
	//	the dstlayer 
	for(auto p: visiblePortals){
		rqis->add(RENDER_QUEUE_PORTAL+p.second->portalId, "Prt"/*+std::to_string(p.first)*/);
	}

	rqis->add(RENDER_QUEUE_PARTICLES, "dummy");

	// Prepare for portal scene drawing by clearing the depth buffer
	rqis->add(1, "PrepPrtScn");

	// Draw each portal scene masked by dstlayer in the stencilbuffer.
	//	Then draw the portal frame 'behind' the portal
	for(auto p: visiblePortals){
		auto scenestr = "PtS"+std::to_string(p.second->portalId);

		rqis->add(RENDER_QUEUE_LAYER+p.first, scenestr);
		// rqis->add(RENDER_QUEUE_PORTALFRAME+p.second->portalId, scenestr);
	}
}

// TODO: Holy shit, clean this mess up
void LayerRenderingManager::renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) {
	auto invocationType = invocation.substr(0,3);
	auto rs = Ogre::Root::getSingleton().getRenderSystem();

	if(invocationType == "Prt"){
		// auto layer = std::stol(invocation.substr(3));
		// auto layer = queueId - RENDER_QUEUE_PORTAL;
		// auto portalId = std::stol(invocation.substr(3));
		
		auto portalId = queueId - RENDER_QUEUE_PORTAL;
		auto portal = PortalManager::GetSingleton()->portals[portalId];
		s32 layer = (portal->layer[0] == (s32)currentLayer)?portal->layer[1]:portal->layer[0];

		if(!portal->shouldDraw) {
			skipThisInvocation = true;
			return;
		}

		rs->setStencilCheckEnabled(true);
		rs->_setColourBufferWriteEnabled(false, false, false, false);
		rs->setStencilBufferParams(
			Ogre::CMPF_ALWAYS_PASS, // compare
			1<<layer, // refvalue
			0xFFFFFFFF, // compare mask
			0xFFFFFFFF, // write mask
			Ogre::SOP_REPLACE, Ogre::SOP_KEEP, // stencil fail, depth fail
			Ogre::SOP_REPLACE, // stencil pass + depth pass
			false); // two-sided operation? no

	}else if(queueId == RENDER_QUEUE_PARTICLES){
		rs->setStencilCheckEnabled(false);
		rs->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS, 0, 0, 0,
			Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP, false);

	}else if(invocationType == "PtS"){
		auto portalId = std::stol(invocation.substr(3));
		auto portal = PortalManager::GetSingleton()->portals[portalId];

		if(!portal->shouldDraw) {
			skipThisInvocation = true;
			return;
		}

		s32 layer = (portal->layer[0] == (s32)currentLayer)?portal->layer[1]:portal->layer[0];
		// auto layer = queueId - RENDER_QUEUE_LAYER;

		rs->_setColourBufferWriteEnabled(true, true, true, true);
		rs->setStencilCheckEnabled(true);
		rs->setStencilBufferParams(Ogre::CMPF_EQUAL, 1<<layer, 0xFFFFFFFF, 0xFFFFFFFF,
			Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP, false);

		// TODO: Make good
		auto playerPos = camera->cameraNode->_getDerivedPosition();
		auto side = portal->clip.getSide(playerPos);

		if(side == Ogre::Plane::NEGATIVE_SIDE){
			rs->addClipPlane(portal->clip);

		}else{
			Ogre::Plane nplane{
				-portal->clip.normal,
				portal->clip.d // I don't know why this works
			};

			rs->addClipPlane(nplane);
		}
	}
}

void LayerRenderingManager::renderQueueEnded(u8 /*queueId*/, const std::string& invocation, bool& /*repeatThisInvocation*/) {
	auto invocationType = invocation.substr(0,3);
	auto rs = Ogre::Root::getSingleton().getRenderSystem();
	rs->setStencilCheckEnabled(false);
	rs->setStencilBufferParams();

	if(invocation == "PrepPrt"){
		// Prepare portal drawing
		rs->clearFrameBuffer(Ogre::FBT_STENCIL, Ogre::ColourValue::Black, 1.0, 0xFF);
	}else if(invocation == "PrepPrtScn"){
		// Prepare portal scene drawing
		rs->clearFrameBuffer(Ogre::FBT_DEPTH, Ogre::ColourValue::Black, 1.0, 0xFF);
	}else if(invocationType == "PtS"){
		rs->resetClipPlanes();
	}
}