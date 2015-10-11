#include <OGRE/OgreParticleSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>

#include <OGRE/OgreManualObject.h>

#include "app.h"
#include "input.h"
#include "player.h"
#include "camera.h"
#include "apptime.h"
#include "audiomanager.h"
#include "portalmanager.h"
#include "synthcomponent.h"
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
	sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(0.1f, 0.1f, 0.1f), 0.05f, 10.0f, 30.0f);
	portalManager = std::make_shared<PortalManager>();

	// OgitorSceneLoader{}.Load("GameData/bend.scene", this);
	// entityManager->entities[1]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<1;
	// entityManager->entities[2]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<0;
	// auto ico = entityManager->entities[0]->AddComponent<SphereColliderComponent>(1.f, true);
	// ico->collisionGroups = 1<<0;

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/TestTemple", "FileSystem");
	BlenderSceneLoader{}.Load("GameData/Scenes/TestTemple/temple.scene", this);

	// Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/Mirror1", "FileSystem");
	// BlenderSceneLoader{}.Load("GameData/Scenes/Mirror1/mirror1.scene", this);

	for(auto& e: entityManager->entities){
		std::cout << "Entity " << e->id << "\tname: " << e->GetName() << "\n";
	}
	std::cout << std::endl;

	auto playerEnt = entityManager->CreateEntity();
	playerEnt->ogreSceneNode = rootNode->createChildSceneNode("Player");
	playerEnt->ogreSceneNode->setPosition(0,2,0);
	camera = playerEnt->AddComponent<Camera>("MainCamera");
	portalManager->SetCamera(camera);

	playerEnt->AddComponent<AudioListenerComponent>();
	entityManager->FindEntity("Door")->AddComponent<SynthComponent>(0);
	entityManager->FindEntity("TrophyRoom")->AddComponent<SynthComponent>(1);
	entityManager->FindEntity("4WayFrame")->AddComponent<SynthComponent>(2);

	player = playerEnt->AddComponent<Player>();
	auto playerCollider = playerEnt->AddComponent<CapsuleColliderComponent>(vec3{2.f, 3.f, 2.f}, true);
	playerCollider->DisableRotation();
	playerCollider->collisionGroups = 1<<0; // First layer

	portalManager->SetLayer(0);

	auto psystem = sceneManager->createParticleSystem("Dust", "Environment/Dust");
	psystem->setRenderQueueGroup(RENDER_QUEUE_PARTICLES);
	camera->cameraNode->attachObject(psystem);

	camera->cameraNode->setPosition(0, 1.4f, 0);
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

	// Return to menu on ESC
	if (Input::GetMappedDown(Input::Cancel)) {
		SetGameState(GameState::MAIN_MENU);
	}

	input->EndFrame();
}

void App::Terminate() {
	ResetScene();
	portalManager.reset();
}