#include <OGRE/OgreRenderQueueInvocation.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRoot.h>

// TODO: Remove unused OGRE headers once layer rendering manager is complete

#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "portalmanager.h"
#include "meshinfo.h"
#include "camera.h"
#include "entity.h"
#include "app.h"

#include <cassert>

Portal::Portal(s32 l0, s32 l1) : Component{this} {
	layer[0] = std::min(l0, l1);
	layer[1] = std::max(l0, l1);
}

template<>
PortalManager* Singleton<PortalManager>::instance = nullptr;

PortalManager::PortalManager() {

}

PortalManager::~PortalManager(){

}

void PortalManager::AddPortal(Portal* portal){
	if(!portal) return;

	auto id = portal->portalId = (s32)portals.size();
	assert(id < 30);

	portals.push_back(portal);

	portal->shouldDraw = true;

	auto& numLayers = App::GetSingleton()->layerRenderingManager->numLayers;
	numLayers = std::max((u32)portal->layer[1]+1, numLayers);
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