#include <OGRE/OgreParticleSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>

#include "app.h"
#include "input.h"
#include "player.h"
#include "camera.h"
#include "apptime.h"
#include "audiomanager.h"
#include "portalmanager.h"
#include "physicsmanager.h"
#include "ogitorsceneloader.h"
#include "blendersceneloader.h"

#include "entity.h"
#include "component.h"
#include "entitymanager.h"

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
	std::cout << "App Init" << std::endl;
	portalManager = std::make_shared<PortalManager>(ogreRoot.get(), camera);
	sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(0.1f, 0.1f, 0.1f), 0.05f, 10.0f, 30.0f);

#if 0
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Meshes", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Particles", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
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
		cnode->translate(0, 0.0, 20.0);
	}

	auto sceneNode1 = rootNode->createChildSceneNode();
	auto portalNode = rootNode->createChildSceneNode();
	portalNode->translate(0, 0, -3.f);

	s32 repetitions = 4;
	for(s32 z = -repetitions; z <= repetitions; z++)
		for(s32 x = -repetitions; x <= repetitions; x++){
			auto cnode = sceneNode1->createChildSceneNode();
			auto courtyard = sceneManager->createEntity("Courtyard.mesh");
			cnode->attachObject(courtyard);
			cnode->scale(0.25, 0.25, 0.25); // 42/4 21 10.5
			cnode->translate(x*21.0, 0.0, z*21.0-2.0);
		}

	auto door = sceneManager->createEntity("mergeDoor.mesh");
	auto doorNode = portalNode->createChildSceneNode();
	doorNode->attachObject(door);
	doorNode->scale(0.35, 0.35, 0.35);
	doorNode->translate(-0.707, 0.0, 0.707);
	doorNode->yaw(Ogre::Radian(M_PI/4.0));

	auto door2 = sceneManager->createEntity("mergeGate.mesh");
	auto door2Node = portalNode->createChildSceneNode();
	door2Node->attachObject(door2);
	door2Node->scale(0.4, 0.4, 0.4);
	door2Node->translate(10.0, 0.0, 0);

	auto l3ent = sceneManager->createEntity("Icosphere.mesh");
	l3ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+2);

	auto l3entNode = rootNode->createChildSceneNode();
	l3entNode->attachObject(l3ent);
	l3entNode->translate(10.0, 1.0, -6.0);

	portalManager->AddPortal(door, 0, 1);
	portalManager->AddPortal(door2, 1, 2);

#else
	// OgitorSceneLoader{}.Load("GameData/bend.scene", this);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/TestTempleScene", "FileSystem");
	BlenderSceneLoader{}.Load("GameData/TestTempleScene/temple.scene", this);

	for(auto& e: entityManager->entities){
		std::cout << "Entity " << e->id << "\tname: " << e->GetName() << "\n";
	}
	std::cout << std::endl;

	// entityManager->entities[1]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<1;
	// entityManager->entities[2]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<0;
	// auto ico = entityManager->entities[0]->AddComponent<SphereColliderComponent>(1.f, true);
	// ico->collisionGroups = 1<<0;
#endif

	auto player = entityManager->CreateEntity();
	player->ogreSceneNode = rootNode->createChildSceneNode();
	player->ogreSceneNode->setPosition(0,2,0);

	// TODO: This should really be an Entity::AddChild
	// Camera should be a component of a child entity
	camera->cameraNode->getParentSceneNode()->removeChild(camera->cameraNode);
	player->ogreSceneNode->addChild(camera->cameraNode);

	player->AddComponent<Player>();
	auto playerCollider = player->AddComponent<CapsuleColliderComponent>(1.f, 1.f, true);
	playerCollider->DisableRotation();
	playerCollider->collisionGroups = 1<<0; // First layer

	auto ground = entityManager->CreateEntity();
	ground->ogreSceneNode = rootNode->createChildSceneNode();
	ground->ogreSceneNode->setPosition(0.0f, -0.52f, 0.0f);
	auto groundcol = ground->AddComponent<BoxColliderComponent>(vec3{1000., 1., 1000.}, false);
	groundcol->collisionGroups = ~0u;

	portalManager->SetLayer(0);
	sceneManager->addRenderQueueListener(portalManager.get());

	auto psystem = sceneManager->createParticleSystem("Dust", "Environment/Dust");
	psystem->setRenderQueueGroup(RENDER_QUEUE_PARTICLES);
	camera->cameraNode->attachObject(psystem);

	camera->cameraNode->setPosition(0, 1.0, 0);
	auto g = 0.1f;
	camera->viewport->setBackgroundColour(Ogre::ColourValue(g, g, g));
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
void App::Update(){
	input->Update();
	entityManager->Update();
	physicsManager->Update();
	audioManager->Update();

	// Close window on ESC
	if(Input::GetKeyDown(SDLK_ESCAPE)){
		shouldQuit = true;
	}

	input->EndFrame();
}

void App::Terminate() {
	//Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}