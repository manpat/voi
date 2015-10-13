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
#include "audiogenerator.h"
#include "soundcomponent.h"
#include "synthcomponent.h"
#include "physicsmanager.h"
#include "ogitorsceneloader.h"
#include "blendersceneloader.h"
#include "areatriggermanager.h"

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

#include "testaudiogenerators.h"

void App::Init(){
	std::cout << "App Init" << std::endl;

	// HACK: Move somewhere good
	static bool audioGeneratorsRegistered = false;
	if(!audioGeneratorsRegistered){
		audioManager->RegisterAudioGeneratorType<DoorAudioGenerator>("door");
		audioManager->RegisterAudioGeneratorType<TrophyAudioGenerator>("trophy");
		audioManager->RegisterAudioGeneratorType<FourWayAudioGenerator>("4way");
		audioManager->RegisterAudioGeneratorType<HighArpeggiatorAudioGenerator>("higharp");
		audioManager->RegisterAudioGeneratorType<LowArpeggiatorAudioGenerator>("lowarp");
		audioManager->RegisterAudioGeneratorType<NoiseAudioGenerator>("noise");
		audioGeneratorsRegistered = true;
	}

	// OgitorSceneLoader{}.Load("GameData/bend.scene", this);
	// entityManager->entities[1]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<1;
	// entityManager->entities[2]->AddComponent<MeshColliderComponent>()->collisionGroups = 1<<0;
	// auto ico = entityManager->entities[0]->AddComponent<SphereColliderComponent>(1.f, true);
	// ico->collisionGroups = 1<<0;

	Load("GameData/Scenes/temple/temple.scene");

	// Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/mirror1", "FileSystem");
	// BlenderSceneLoader{}.Load("GameData/Scenes/mirror1/mirror1.scene", this);

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
	areaTriggerManager->Update();

	input->Update();
	entityManager->Update();
	physicsManager->Update();
	audioManager->Update();
	entityManager->LateUpdate();

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

/*
	                                               
	88                                         88  
	88                                         88  
	88                                         88  
	88          ,adPPYba,  ,adPPYYba,  ,adPPYb,88  
	88         a8"     "8a ""     `Y8 a8"    `Y88  
	88         8b       d8 ,adPPPPP88 8b       88  
	88         "8a,   ,a8" 88,    ,88 "8a,   ,d88  
	88888888888 `"YbbdP"'  `"8bbdP"Y8  `"8bbdP"Y8  
	                                               
	                                               
*/
void App::Load(const std::string& nLevel){
	Terminate();

	auto slash = nLevel.find_last_of("/\\");
	auto path = nLevel.substr(0, slash);

	sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(0.1f, 0.1f, 0.1f), 0.05f, 10.0f, 30.0f);
	portalManager = std::make_shared<PortalManager>();

	// Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/temple", "FileSystem");
	std::cout << path << std::endl;
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(path, "FileSystem");
	BlenderSceneLoader{}.Load(nLevel, this);

	for(auto& e: entityManager->entities){
		std::cout << "Entity " << e->id << "\tname: " << e->GetName() << "\n";
	}
	std::cout << std::endl;

	auto playerEnt = entityManager->CreateEntity("Player", vec3{0,2,0});
	camera = playerEnt->AddComponent<Camera>("MainCamera");
	portalManager->SetCamera(camera);

	playerEnt->AddComponent<AudioListenerComponent>();

	player = playerEnt->AddComponent<Player>();
	auto playerCollider = playerEnt->AddComponent<CapsuleColliderComponent>(vec3{2.f, 3.f, 2.f}, true);
	playerCollider->DisableRotation();
	playerEnt->SetLayer(0);

	auto psystem = sceneManager->createParticleSystem("Dust", "Environment/Dust");
	psystem->setRenderQueueGroup(RENDER_QUEUE_PARTICLES);
	camera->cameraNode->attachObject(psystem);

	camera->cameraNode->setPosition(0, 1.4f, 0);
	auto g = 0.1f;
	camera->viewport->setBackgroundColour(Ogre::ColourValue(g, g, g));
}