#include <OGRE/OgreParticleSystemManager.h>
#include <OGRE/OgreParticleSystem.h>
#include <OGRE/OgreSceneManager.h>
// #include <OGRE/OgreManualObject.h>

#include "app.h"
#include "input.h"
#include "player.h"
#include "camera.h"
#include "cursor.h"
#include "apptime.h"
#include "bellmanager.h"
#include "audiomanager.h"
#include "portalmanager.h"
#include "mirrormanager.h"
#include "audiogenerator.h"
#include "soundcomponent.h"
#include "synthcomponent.h"
#include "physicsmanager.h"
#include "blendersceneloader.h"
#include "areatriggermanager.h"
#include "layerrenderingmanager.h"

#include "entity.h"
#include "component.h"
#include "entitymanager.h"

#include <chrono>

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

#include "AudioGenerators/testaudiogenerators.h"
#include "AudioGenerators/ambience.h"
#include "AudioGenerators/hub.h"

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
		audioManager->RegisterAudioGeneratorType<LowRumbleAudioGenerator>("lowrumble");

		audioManager->RegisterAudioGeneratorType<HubAudioGenerator>("hub");
		BellManager::RegisterAudio();

		audioGeneratorsRegistered = true;
	}

	Load(!customLevelName.empty() ? customLevelName : "hub");
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

	if(Input::GetKeyDown(SDLK_F2)) {
		Input::doCapture = !Input::doCapture;
	}

	input->EndFrame();
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
	using ck = std::chrono::high_resolution_clock;
	auto lbegin = ck::now();

	Terminate();

	auto lvl = nLevel + ".scene";
	SceneFileInfo* sceneInfo = nullptr;
	for(auto& si: scenes) {
		if(si.name == lvl){
			sceneInfo = &si;
			break;
		}
	}

	if(!sceneInfo){
		throw "Scene " + nLevel + " not found";
	}

	// sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(0.1f, 0.1f, 0.1f), 0.05f, 10.0f, 30.0f);
	// sceneManager->setFog(Ogre::FOG_EXP, Ogre::ColourValue(0.1f, 0.1f, 0.1f), 0.05f, 20.0f, 40.0f);
	layerRenderingManager = std::make_shared<LayerRenderingManager>();
	portalManager = std::make_shared<PortalManager>();
	mirrorManager = std::make_shared<MirrorManager>();
	bellManager = std::make_shared<BellManager>();

	// Reset to defaults in case a level doesn't have a spawn point
	playerSpawnOrientation = quat::IDENTITY;
	playerSpawnPosition = vec3::ZERO;

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(sceneInfo->path, "FileSystem");
	BlenderSceneLoader scnLdr{};
	scnLdr.Load(sceneInfo->path+sceneInfo->name, this);
	auto& env = scnLdr.environment;

	const auto playerHeight = 3.f;
	const auto playerCenter = playerHeight/2.f;

	playerSpawnPosition += vec3{0, playerCenter, 0};

	// Created at half height above ground because collider origins are from the center
	auto playerEnt = entityManager->CreateEntity("Player", playerSpawnPosition);
	auto cameraEnt = entityManager->CreateEntity("Camera");
	playerEnt->AddChild(cameraEnt);
	cameraEnt->SetGlobalPosition(playerSpawnPosition + vec3{0, playerCenter-0.2f, 0});

	camera = cameraEnt->AddComponent<Camera>("MainCamera");
	layerRenderingManager->SetCamera(camera);

	player = playerEnt->AddComponent<Player>();
	// NOTE: Orientation doesn't seem to work here
	player->SetToOrientation(playerSpawnOrientation);
	
	auto playerCollider = playerEnt->AddComponent<CapsuleColliderComponent>(vec3{2.f, playerHeight, 2.f}, true);
	playerCollider->DisableRotation();
	playerEnt->SetLayer(0);

	playerEnt->AddComponent<AudioListenerComponent>();

	auto psystem = sceneManager->createParticleSystem("Dust", "Environment/Dust");
	psystem->setRenderQueueGroup(RENDER_QUEUE_PARTICLES);
	camera->cameraNode->attachObject(psystem);

	layerRenderingManager->SetupRenderQueueInvocationSequence(0);

	// Set up environment parameters
	sceneManager->setFog(
		(Ogre::FogMode)env.fogType,
		Ogre::ColourValue{env.fogColor[0], env.fogColor[1], env.fogColor[2]},
		env.fogDensity,
		env.fogStart, env.fogEnd);

	skyColor = Ogre::ColourValue{env.skyColor[0], env.skyColor[1], env.skyColor[2]};
	camera->viewport->setBackgroundColour(skyColor);
	portalManager->SetPortalColor(skyColor);

	// TODO: Do something with environment.ambientColor

	// Make everything shadeless
	auto matIt = Ogre::MaterialManager::getSingletonPtr()->getResourceIterator();
	while(matIt.hasMoreElements()) {
		auto res = matIt.current()->second;
		auto mat = static_cast<Ogre::Material*>(res.get());
		if(mat->getName() == "Portal") {
			matIt.moveNext();
			continue;
		}

		mat->setShadingMode(Ogre::SO_FLAT);

		auto pass = mat->getTechnique(0)->getPass(0);
		pass->setEmissive(pass->getDiffuse());

		matIt.moveNext();
	}

	cursor = new Cursor();

	auto lend = ck::now();
	auto diff = std::chrono::duration_cast<std::chrono::duration<f32>> (lend-lbegin).count();
	std::cout << "Scene load for " << nLevel << " took " << diff << "s" << std::endl;
}

/*

	88888888ba
	88      "8b                                  ,d
	88      ,8P                                  88
	88aaaaaa8P' ,adPPYba, ,adPPYba,  ,adPPYba, MM88MMM
	88""""88'  a8P_____88 I8[    "" a8P_____88   88
	88    `8b  8PP"""""""  `"Y8ba,  8PP"""""""   88
	88     `8b "8b,   ,aa aa    ]8I "8b,   ,aa   88,
	88      `8b `"Ybbd8"' `"YbbdP"'  `"Ybbd8"'   "Y888


*/
void App::ResetScene() {
	auto defaultResGrp = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;

	// Destroy all existing entities
	entityManager->DestroyAllEntities();

	// Clear scene completely
	sceneManager->clearScene();

	// Destroy particle systems and particle templates
	sceneManager->destroyAllParticleSystems();
	Ogre::ParticleSystemManager::getSingleton().removeAllTemplates();

	// Clear default resouce group resources
	Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(defaultResGrp);

	// Remove default resouce group resource locations
	Ogre::ResourceGroupManager::LocationList locList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(defaultResGrp);

	for (auto l = locList.begin(); l != locList.end(); l++) {
		Ogre::ResourceGroupManager::getSingleton().removeResourceLocation((*l)->archive->getName(), defaultResGrp);
	}
}

void App::Terminate() {
	ResetScene();
	bellManager.reset();
	portalManager.reset();
	mirrorManager.reset();
	layerRenderingManager.reset();
}