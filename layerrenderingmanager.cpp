#include <OGRE/OgreRenderQueueInvocation.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreFrustum.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreRoot.h>

#include "layerrenderingmanager.h"
#include "portalmanager.h"
#include "mirrormanager.h"
#include "camera.h"
#include "entity.h"
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

	// List of visible mirrors
	std::vector<Mirror*> visibleMirrors;

	// Collect list of visible mirrors
	for (auto m: MirrorManager::GetSingleton()->mirrors) {
		// If mirror is in this layer, add to list of visible mirrors.
		if (m->layer == l) {
			visibleMirrors.push_back(m);
		}
	}

	// Prepare for portal/mirror drawing by clearing the stencil buffer
	rqis->add(1, "Prep");

	// Draw each portal to the stencil buffer with a ref value of 
	//	the dstlayer 
	for(auto p: visiblePortals){
		rqis->add(RENDER_QUEUE_PORTAL+p.second->portalId, "Prt"/*+std::to_string(p.first)*/);
	}

	// Add visible mirrors to render sequence for drawing to stencil buffer
	for (auto m: visibleMirrors) {
		rqis->add(RENDER_QUEUE_MIRRORED + m->mirrorId, "Mir");
	}

	rqis->add(RENDER_QUEUE_PARTICLES, "dummy");

	// Prepare for portal/mirror scene drawing by clearing the depth buffer
	rqis->add(1, "PrepScn");

	// Draw each portal scene masked by dstlayer in the stencilbuffer.
	//	Then draw the portal frame 'behind' the portal
	for(auto p: visiblePortals){
		auto scenestr = "PtS"+std::to_string(p.second->portalId);

		rqis->add(RENDER_QUEUE_LAYER+p.first, scenestr);
		// rqis->add(RENDER_QUEUE_PORTALFRAME+p.second->portalId, scenestr);
	}

	// Add visible mirrors to render sequence for drawing of reflected scene
	for (auto m: visibleMirrors) {
		auto scenestr = "MiS" + std::to_string(m->mirrorId);
		rqis->add(RENDER_QUEUE_LAYER + l, scenestr);
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
	} else if (invocationType == "Mir") {
		auto mirrorId = queueId - RENDER_QUEUE_MIRRORED;
		auto mirror = MirrorManager::GetSingleton()->mirrors[mirrorId];
	
		if (!mirror->isVisible) {
			skipThisInvocation = true;
			return;
		}

		rs->setStencilCheckEnabled(true);
		rs->setStencilBufferParams(
			Ogre::CMPF_ALWAYS_PASS, // func
			1 + mirrorId, // refValue
			0xFF, // compareMask
			0xFF, // writeMask
			Ogre::SOP_KEEP, // stencilFailOp
			Ogre::SOP_KEEP, // depthFailOp
			Ogre::SOP_REPLACE, // passOp
			false); // twoSidedOperation
	} else if (invocationType == "MiS") {
		auto mirrorId = std::stol(invocation.substr(3));
		auto mirror = MirrorManager::GetSingleton()->mirrors[mirrorId];

		rs->setStencilCheckEnabled(true);
		rs->setStencilBufferParams(
			Ogre::CMPF_EQUAL, // func
			1 + mirrorId, // refValue
			0xFF, // compareMask
			0x00, // writeMask
			Ogre::SOP_KEEP, // stencilFailOp
			Ogre::SOP_KEEP, // depthFailOp
			Ogre::SOP_KEEP, // passOp
			false); // twoSidedOperation

		//mirror->cullFrustum.setCustomViewMatrix(true, camera->ogreCamera->getViewMatrix() * mirror->reflectionMat);
		//camera->ogreCamera->setCullingFrustum(&mirror->cullFrustum);
		camera->ogreCamera->setCustomViewMatrix(true, camera->ogreCamera->getViewMatrix() * mirror->reflectionMat);

		// Set render system view matrix to match camera view matrix as per update in render queue ended
		rs->_setViewMatrix(camera->ogreCamera->getViewMatrix());
		rs->setInvertVertexWinding(true);
		//rs->_setCullingMode(Ogre::CULL_ANTICLOCKWISE);
		rs->addClipPlane(mirror->clipPlane);
		//camera->ogreCamera->enableReflection(mirror->clipPlane);
		//camera->ogreCamera->enableCustomNearClipPlane(mirror->clipPlane);
	}
}

void LayerRenderingManager::renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) {
	auto invocationType = invocation.substr(0,3);
	auto rs = Ogre::Root::getSingleton().getRenderSystem();

	rs->setStencilCheckEnabled(false);
	rs->setStencilBufferParams();

	if(invocation == "Prep"){
		// Prepare portal/mirror drawing by clearing the stencil buffer
		rs->clearFrameBuffer(Ogre::FBT_STENCIL, Ogre::ColourValue::Black, 1.0, 0xFF);
	}else if(invocation == "PrepScn"){
		// Prepare portal/mirror scene drawing by clearing the depth buffer
		rs->clearFrameBuffer(Ogre::FBT_DEPTH, Ogre::ColourValue::Black, 1.0, 0xFF);
	}else if(invocationType == "PtS"){
		// Post drawing of portal scene
		rs->resetClipPlanes();
	} else if (invocationType == "MiS") {
		// Post drawing of mirror scene
		camera->ogreCamera->setCustomViewMatrix(false);
		//camera->ogreCamera->setCullingFrustum(0);
		rs->setInvertVertexWinding(false);
		rs->resetClipPlanes();
	}
}