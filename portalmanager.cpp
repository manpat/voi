#include <OGRE/OgreRenderQueueInvocation.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRoot.h>
#include "portalmanager.h"
#include "camera.h"

#include <cassert>

PortalManager::PortalManager(Ogre::Root* root, std::shared_ptr<Camera>& c)
	: camera(c){

	rqis = root->createRenderQueueInvocationSequence("Lol");
	camera->viewport->setRenderQueueInvocationSequenceName("Lol");

	numLayers = 3;
	SetLayer(0);
}

void PortalManager::renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) {
	auto invocationType = invocation.substr(0,3);
	auto rs = Ogre::Root::getSingleton().getRenderSystem();

	if(invocationType == "Prt"){
		auto layer = std::stol(invocation.substr(3));

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
		auto& portal = portals[portalId];
		auto layer = (s32)queueId - RENDER_QUEUE_PORTALSCENE;

		if(layer < 0){
			layer = (s32)queueId - RENDER_QUEUE_PORTALFRAME;
		}

		rs->_setColourBufferWriteEnabled(true, true, true, true);
		rs->setStencilCheckEnabled(true);
		rs->setStencilBufferParams(Ogre::CMPF_EQUAL, 1<<layer, 0xFFFFFFFF, 0xFFFFFFFF,
			Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP, false);

		// TODO: Make good
		auto playerPos = camera->cameraNode->getPosition();
		auto side = portal.clip.getSide(playerPos);

		if(side == Ogre::Plane::NEGATIVE_SIDE){
			rs->addClipPlane(portal.clip);

		}else{
			Ogre::Plane nplane{
				-portal.clip.normal,
				portal.clip.d // I don't know why this works
			};

			rs->addClipPlane(nplane);
		}

	}
}

void PortalManager::renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) {
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

void PortalManager::SetLayer(s32 l){
	assert(l < numLayers);

	rqis->clear();
	rqis->add(RENDER_QUEUE_PORTALSCENE+l, "Main");

	using PType = std::pair<s32, Portal*>;
	static std::vector<PType> visiblePortals;
	visiblePortals.clear();

	for(auto& p: portals){
		s32 layer = -1;
		if(p.layer[0] == l){
			layer = p.layer[1];

		}else if(p.layer[1] == l){
			layer = p.layer[0];
		}

		if(layer != -1){
			rqis->add(RENDER_QUEUE_PORTALFRAME+p.id, "Main");
			visiblePortals.push_back(PType(layer, &p));
		}
	}

	rqis->add(1, "PrepPrt");

	for(auto p: visiblePortals){
		rqis->add(RENDER_QUEUE_PORTAL+p.second->id, "Prt"+std::to_string(p.first));
	}

	rqis->add(RENDER_QUEUE_PARTICLES, "");
	rqis->add(1, "PrepPrtScn");

	for(auto p: visiblePortals){
		auto scenestr = "PtS"+std::to_string(p.second->id);

		rqis->add(RENDER_QUEUE_PORTALSCENE+p.first, scenestr);
		rqis->add(RENDER_QUEUE_PORTALFRAME+p.first, scenestr);
	}
	
}

s32 PortalManager::AddPortal(Ogre::SubEntity* ent, s32 l0, s32 l1){
	auto id = (s32)portals.size();
	assert(id < 10);

	ent->getParent()->setRenderQueueGroup(RENDER_QUEUE_PORTALFRAME+id);
	ent->setRenderQueueGroup(RENDER_QUEUE_PORTAL+id);

	auto portalNode = ent->getParent()->getParentSceneNode();
	// This is not the best but it's good enough for now
	auto pos = portalNode->_getDerivedPosition();
	auto ori = portalNode->_getDerivedOrientation();
	auto normal = ori * vec3::UNIT_Z;
	normal.normalise();

	auto length = pos.dotProduct(normal);
	Ogre::Plane clip{normal, length};

	portals.push_back(Portal{
		id,
		{std::min(l0, l1),	std::max(l0, l1)},
		clip
	});

	return id;
}