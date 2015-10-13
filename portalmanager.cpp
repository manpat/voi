#include <OGRE/OgreRenderQueueInvocation.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRoot.h>
#include "portalmanager.h"
#include "physicsmanager.h"
#include "meshinfo.h"
#include "camera.h"
#include "entity.h"
#include "app.h"

#include <cassert>

Portal::Portal(s32 l0, s32 l1) : Component{this} {
	layer[0] = std::min(l0, l1);
	layer[1] = std::max(l0, l1);
}

PortalManager::PortalManager() {
	rqis = App::GetSingleton()->ogreRoot->createRenderQueueInvocationSequence("PortalInvocationSequence");
	App::GetSingleton()->sceneManager->addRenderQueueListener(this);

	numLayers = 1;
	SetLayer(0);
}

PortalManager::~PortalManager(){
	App::GetSingleton()->ogreRoot->destroyRenderQueueInvocationSequence("PortalInvocationSequence");
	App::GetSingleton()->sceneManager->removeRenderQueueListener(this);
	rqis = nullptr;
}

// TODO: Remove all traces of portal frames
// TODO: Holy shit, clean this mess up
void PortalManager::renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) {
	auto invocationType = invocation.substr(0,3);
	auto rs = Ogre::Root::getSingleton().getRenderSystem();

	if(invocationType == "Prt"){
		// auto layer = std::stol(invocation.substr(3));
		// auto layer = queueId - RENDER_QUEUE_PORTAL;
		// auto portalId = std::stol(invocation.substr(3));
		
		auto portalId = queueId - RENDER_QUEUE_PORTAL;
		auto portal = portals[portalId];
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
		auto portal = portals[portalId];

		if(!portal->shouldDraw) {
			skipThisInvocation = true;
			return;
		}

		s32 layer = (portal->layer[0] == (s32)currentLayer)?portal->layer[1]:portal->layer[0];
		// auto layer = queueId - RENDER_QUEUE_PORTALSCENE;

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

void PortalManager::renderQueueEnded(u8 /*queueId*/, const std::string& invocation, bool& /*repeatThisInvocation*/) {
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

	for(auto p: portals){
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

		rqis->add(RENDER_QUEUE_PORTALSCENE+p.first, scenestr);
		// rqis->add(RENDER_QUEUE_PORTALFRAME+p.second->portalId, scenestr);
	}
}

void PortalManager::AddPortal(Portal* portal){
	if(!portal) return;

	auto id = portal->portalId = (s32)portals.size();
	assert(id < 30);

	portals.push_back(portal);

	// portal->shouldDraw = false;
	portal->shouldDraw = true;
	numLayers = std::max((u32)portal->layer[1]+1, numLayers);
}

void PortalManager::SetCamera(Camera* c){
	camera = c;
	camera->viewport->setRenderQueueInvocationSequenceName("PortalInvocationSequence");
}

void Portal::OnInit(){
	auto prtMgr = App::GetSingleton()->portalManager;

	assert(layer[0] < 10 && layer[1] < 10);
	prtMgr->AddPortal(this);

	// Get subentity with name Portal
	Ogre::SubEntity* portalSubEnt = nullptr;
	Ogre::SubMesh* portalSubmesh = nullptr;

	assert(entity->ogreEntity);
	auto portalMesh = entity->ogreEntity->getMesh();

	auto subMeshes = portalMesh->getSubMeshNameMap(); // std::unordered_map<string, ushort>
	auto subMeshIt = subMeshes.find("Portal"); // Find submesh with name Portal
	if(subMeshIt == subMeshes.end()) { // If failed
		// Get submesh with material of name Portal
		for(auto& sm: subMeshes){ 
			if(portalMesh->getSubMesh(sm.second)->getMaterialName() == "Portal"){
				portalSubEnt = entity->ogreEntity->getSubEntity(sm.second);
				break;
			}
		}

		// No portal surface found
		if(!portalSubEnt) {
			std::cout << "No portal surfaces found in " << entity->GetName() << std::endl;
			return;
		}
	}else{
		//
		portalSubEnt = entity->ogreEntity->getSubEntity(subMeshIt->second);
	}

	portalSubmesh = portalSubEnt->getSubMesh();

	// This assumes that subMeshes and subEntities match one to one
	portalSubEnt->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.1f, 0.1f, 0.1f)); // Skycolor
	portalSubEnt->getMaterial()->setCullingMode(Ogre::CULL_NONE); // Draw back and front face
	portalSubEnt->getMaterial()->setDepthBias(10.f, 0.f);

	// ent->setRenderQueueGroup(RENDER_QUEUE_PORTALFRAME+id);
	portalSubEnt->setRenderQueueGroup(RENDER_QUEUE_PORTAL+portalId);

	auto mesh = GetOgreSubMeshVertices(portalSubmesh);

	auto forward = vec3::UNIT_Z;
	auto posOffset = vec3::ZERO;
	for(u32 i = 0; i < mesh.size()-2; i++){
		posOffset = mesh[i];
		auto p1 = mesh[i+1] - posOffset;
		auto p2 = mesh[i+2] - posOffset;

		auto cross = p1.crossProduct(p2);
		if(cross.length() > 0.9){
			forward = cross.normalisedCopy();
			break;
		}
	}

	auto portalNode = entity->ogreEntity->getParentSceneNode();
	auto pos = portalNode->_getDerivedPosition();
	auto ori = portalNode->_getDerivedOrientation();
	auto normal = ori * forward;
	normal.normalise();

	auto length = pos.dotProduct(normal);
	clip = Ogre::Plane{normal, length};
}