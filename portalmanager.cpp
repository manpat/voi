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

	numLayers = 1;
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
		s32 layer = (portal.layer[0] == (s32)currentLayer)?portal.layer[1]:portal.layer[0];

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
	assert(l < (s32)numLayers);
	currentLayer = l;

	rqis->clear();
	rqis->add(RENDER_QUEUE_PORTALSCENE+l, "Main");

	using PType = std::pair<s32, Portal*>;
	static std::vector<PType> visiblePortals;
	visiblePortals.clear();

	for(auto& p: portals){
		// Test if portal exists in the new layer and if so
		//	get it's destination layer
		s32 layer = -1;
		if(p.layer[0] == l){
			layer = p.layer[1];

		}else if(p.layer[1] == l){
			layer = p.layer[0];
		}

		// if the portal IS in the new layer
		if(layer != -1){
			// Queue it's frame to be rendered
			rqis->add(RENDER_QUEUE_PORTALFRAME+p.id, "Main");
			visiblePortals.push_back(PType(layer, &p));
		}
	}

	// Prepare for portal drawing by clearing the stencil buffer
	rqis->add(1, "PrepPrt");

	// Draw each portal to the stencil buffer with a ref value of 
	//	the dstlayer 
	for(auto p: visiblePortals){
		rqis->add(RENDER_QUEUE_PORTAL+p.second->id, "Prt"+std::to_string(p.first));
	}

	rqis->add(RENDER_QUEUE_PARTICLES, "");

	// Prepare for portal scene drawing by clearing the depth buffer
	rqis->add(1, "PrepPrtScn");

	// Draw each portal scene masked by dstlayer in the stencilbuffer.
	//	Then draw the portal frame 'behind' the portal
	for(auto p: visiblePortals){
		auto scenestr = "PtS"+std::to_string(p.second->id);

		rqis->add(RENDER_QUEUE_PORTALSCENE+p.first, scenestr);
		rqis->add(RENDER_QUEUE_PORTALFRAME+p.second->id, scenestr);
	}
}

void PortalManager::AddPortal(Ogre::Entity* ent, s32 l0, s32 l1){
	auto id = (s32)portals.size();
	assert(id < 10);

	// Get subentity with name Portal
	Ogre::SubEntity* portalEnt = nullptr;
	auto subMeshes = ent->getMesh()->getSubMeshNameMap(); // std::unordered_map<string, ushort>
	auto subMeshIt = subMeshes.find("Portal");
	if(subMeshIt == subMeshes.end()) {
		// No portal surface found
		return;
	}

	// This assumes that subMeshes and subEntities match one to one
	portalEnt = ent->getSubEntity(subMeshIt->second);
	portalEnt->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.1, 0.1, 0.1)); // Skycolor
	portalEnt->getMaterial()->setCullingMode(Ogre::CULL_NONE); // Back and front face

	ent->setRenderQueueGroup(RENDER_QUEUE_PORTALFRAME+id);
	portalEnt->setRenderQueueGroup(RENDER_QUEUE_PORTAL+id);

	auto portalNode = ent->getParentSceneNode();
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

	numLayers = std::max((u32)std::max(l0, l1)+1, numLayers);
}