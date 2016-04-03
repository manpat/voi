#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>

#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "portalmanager.h"
#include "meshinfo.h"
#include "camera.h"
#include "entity.h"
#include <cassert>

Portal::Portal(s32 l0, s32 l1) : Component{this} {
	layer[0] = std::min(l0, l1);
	layer[1] = std::max(l0, l1);
}

template<>
PortalManager* Singleton<PortalManager>::instance = nullptr;

PortalManager::PortalManager() {
	auto& matman = Ogre::MaterialManager::getSingleton();
	portalMaterial = matman.create("Portal",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		true /* Manual material */);

	portalMaterial->setAmbient(Ogre::ColourValue(0.0f, 0.0f, 0.0f)); // Skycolor
	portalMaterial->setCullingMode(Ogre::CULL_NONE); // Draw back and front face
	portalMaterial->setDepthBias(10.f, 0.f);
}

PortalManager::~PortalManager(){

}

void PortalManager::AddPortal(Portal* portal){
	if(!portal) return;

	auto id = portal->portalId = (s32)portals.size();
	assert(id < 30);

	portals.push_back(portal);

	portal->shouldDraw = true;

	auto& numLayers = LayerRenderingManager::GetSingleton()->numLayers;
	numLayers = std::max((u32)portal->layer[1]+1, numLayers);
}

void PortalManager::SetPortalColor(const Ogre::ColourValue& col){
	portalMaterial->setSelfIllumination(col);
}

void Portal::OnInit(){
	auto prtMgr = PortalManager::GetSingleton();

	assert(layer[0] < 10 && layer[1] < 10);
	prtMgr->AddPortal(this);

	// Get subentity with name Portal
	Ogre::SubEntity* portalSubEnt = nullptr;
	Ogre::SubMesh* portalSubmesh = nullptr;

	assert(entity->ogreEntity);
	auto portalMesh = entity->ogreEntity->getMesh();

	portalSubEnt = entity->ogreEntity->getSubEntity(0);
	portalSubmesh = portalSubEnt->getSubMesh();

	// This assumes that subMeshes and subEntities match one to one
	portalSubEnt->setMaterial(prtMgr->portalMaterial);
	portalSubEnt->setRenderQueueGroup(RENDER_QUEUE_PORTAL+portalId);

	auto mesh = GetOgreSubMeshVertices(portalSubmesh);

	auto forward = vec3::UNIT_Z;
	auto posOffset = vec3::ZERO;
	f32 fwdlength = 0.9f;
	for(u32 i = 0; i < mesh.size()-2; i++){
		posOffset = mesh[i];
		auto p1 = mesh[i+1] - posOffset;
		auto p2 = mesh[i+2] - posOffset;

		auto cross = p1.crossProduct(p2);
		if(cross.length() > fwdlength){
			fwdlength = cross.length();
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