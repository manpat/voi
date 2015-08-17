#include <OGRE/OgreParticleSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>

#include "app.h"
#include "input.h"
#include "camera.h"
#include "sceneparser.h"
#include "portalmanager.h"

/*

	88             88
	88             ""   ,d
	88                  88
	88 8b,dPPYba,  88 MM88MMM
	88 88P'   `"8a 88   88
	88 88       88 88   88
	88 88       88 88   88,
	88 88       88 88   "Y888


*/
void App::Init(){
	portalManager = std::make_shared<PortalManager>(ogreRoot.get(), camera);
	sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(.1,.1,.1), 0.05, 10.0, 30.0);

#if 0
	{
		auto spread = 0.8f;
		auto sceneNode2 = rootNode->createChildSceneNode();
		for(s32 y = 0; y <= 10; y++)
			for(s32 x = 0; x <= 10; x++){
				auto ent = sceneManager->createEntity("Icosphere.mesh");
				ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
				auto entNode = sceneNode2->createChildSceneNode();
				entNode->attachObject(ent);
				entNode->scale(0.3f, 0.3f, 0.3f);
				entNode->translate((x-5)*spread, (y-5)*spread, -10.f);
			}

		auto ent = sceneManager->createEntity("Icosphere.mesh");
		ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
		auto entNode = sceneNode2->createChildSceneNode();
		entNode->attachObject(ent);
		entNode->scale(2.f, 2.f, 2.f);
		entNode->translate(0, 0, 5.f);

		auto cnode = sceneNode2->createChildSceneNode();
		auto courtyard = sceneManager->createEntity("Courtyard2", "Courtyard.mesh");
		courtyard->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
		cnode->attachObject(courtyard);
		cnode->scale(0.25, 0.25, 0.25);
		cnode->translate(0, -1.0, 20.0);
	}

	auto sceneNode1 = rootNode->createChildSceneNode();
	auto portalNode = rootNode->createChildSceneNode();
	portalNode->translate(0, 0, -3.f);

	constexpr s32 repetitions = 4;
	for(s32 z = -repetitions; z <= repetitions; z++)
		for(s32 x = -repetitions; x <= repetitions; x++){
			auto cnode = sceneNode1->createChildSceneNode();
			auto courtyard = sceneManager->createEntity("Courtyard.mesh");
			cnode->attachObject(courtyard);
			cnode->scale(0.25, 0.25, 0.25); // 42/4 21 10.5
			cnode->translate(x*21.0, -1.0, z*21.0-2.0);
		}

	auto door = sceneManager->createEntity("mergeDoor.mesh");
	auto doorNode = portalNode->createChildSceneNode();
	doorNode->attachObject(door);
	doorNode->scale(0.35, 0.35, 0.35);
	doorNode->translate(-0.707, -1.0, 0.707	);
	doorNode->yaw(Ogre::Radian(M_PI/4.0));

	auto door2 = sceneManager->createEntity("mergeGate.mesh");
	auto door2Node = portalNode->createChildSceneNode();
	door2Node->attachObject(door2);
	door2Node->scale(0.4, 0.4, 0.4);
	door2Node->translate(10.0, -1.0, 0);

	auto l3ent = sceneManager->createEntity("Icosphere.mesh");
	l3ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+2);

	auto l3entNode = rootNode->createChildSceneNode();
	l3entNode->attachObject(l3ent);
	l3entNode->translate(10.0, 0.0, -6.0);

	Portalify(door, 0, 1);
	Portalify(door2, 1, 2);
#endif

	SceneParser sceneloader;
	sceneloader.Load("TestScene.scene", sceneManager);

	sceneManager->addRenderQueueListener(portalManager.get());
	portalManager->SetLayer(0);

	auto psystem = sceneManager->createParticleSystem("Dust", "Environment/Dust");
	psystem->setRenderQueueGroup(RENDER_QUEUE_PARTICLES);
	camera->cameraNode->attachObject(psystem);
}

/*

	88        88                      88
	88        88                      88              ,d
	88        88                      88              88
	88        88 8b,dPPYba,   ,adPPYb,88 ,adPPYYba, MM88MMM ,adPPYba,
	88        88 88P'    "8a a8"    `Y88 ""     `Y8   88   a8P_____88
	88        88 88       d8 8b       88 ,adPPPPP88   88   8PP"""""""
	Y8a.    .a8P 88b,   ,a8" "8a,   ,d88 88,    ,88   88,  "8b,   ,aa
	 `"Y8888Y"'  88`YbbdP"'   `"8bbdP"Y8 `"8bbdP"Y8   "Y888 `"Ybbd8"'
	             88
	             88
*/
void App::Update(f32 dt){
	auto md = Input::GetMouseDelta();

	auto nyaw =  -md.x * 2.0 * M_PI * dt * 7.f;
	auto npitch = md.y * 2.0 * M_PI * dt * 7.f;
	if(abs(camera->cameraPitch + npitch) < M_PI/4.0f) { // Convoluted clamp
		camera->cameraPitch += npitch;
	}
	camera->cameraYaw += nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(camera->cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(camera->cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->setOrientation(ori);

	f32 boost = 2.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost = 4.f;
	}

	// Move with WASD, based on look direction
	if(Input::GetKey(SDLK_w)){
		camera->cameraNode->translate(-oriYaw.zAxis() * dt * boost);
	}else if(Input::GetKey(SDLK_s)){
		camera->cameraNode->translate(oriYaw.zAxis() * dt * boost);
	}

	if(Input::GetKey(SDLK_a)){
		camera->cameraNode->translate(-oriYaw.xAxis() * dt * boost);
	}else if(Input::GetKey(SDLK_d)){
		camera->cameraNode->translate(oriYaw.xAxis() * dt * boost);
	}

	// Close window on ESC
	if(Input::GetKeyDown(SDLK_ESCAPE)){
		shouldQuit = true;
		return;
	}

	if(Input::GetKeyDown('f')){
		static bool flipped = false;
		portalManager->SetLayer((s32)(flipped = !flipped));
	}
}

void App::Portalify(Ogre::Entity* e, s32 l0, s32 l1){
	/////// The following process should happen during scene loading
	// subMeshes is an std::unordered_map<string, ushort>
	auto subMeshes = e->getMesh()->getSubMeshNameMap();
	auto doorpit = subMeshes.find("Portal");
	if(doorpit == subMeshes.end()) {
		std::cout << "No portal surface found" << std::endl;

		return;
	}else{
		// This assumes that subMeshes and subEntities match one to one
		auto portalEnt = e->getSubEntity(doorpit->second);
		portalEnt->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.1, 0.1, 0.1)); // Skycolor
		portalEnt->getMaterial()->setCullingMode(Ogre::CULL_NONE); // Back and front face

		portalManager->AddPortal(portalEnt, l0, l1);
	}
	////////////////////////////////////////////////////////
}