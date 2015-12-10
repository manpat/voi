#include <OGRE/OgreParticleSystemManager.h>
#include <OGRE/OgreParticleSystem.h>
#include <OGRE/OgreSceneManager.h>
// #include <OGRE/OgreManualObject.h>

#include "app.h"
#include "input.h"
#include "player.h"
#include "camera.h"
#include "apptime.h"
#include "uiimage.h"
#include "uimanager.h"
#include "hubmanager.h"
#include "bellmanager.h"
#include "audiomanager.h"
#include "portalmanager.h"
#include "mirrormanager.h"
#include "audiogenerator.h"
#include "soundcomponent.h"
#include "synthcomponent.h"
#include "physicsmanager.h"
#include "blendersceneloader.h"
#include "endtriggercomponent.h"
#include "halflifepointmanager.h"
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

void InitAudioGenerators(std::shared_ptr<AudioManager> audioManager);

void App::Init(){
	std::cout << "App Init" << std::endl;

	// HACK: Move somewhere good
	static bool audioGeneratorsRegistered = false;
	if(!audioGeneratorsRegistered){
		InitAudioGenerators(audioManager);
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
	auto gameState = GetGameState();

	// Pause or return to menu on ESC
	if (Input::GetMappedDown(Input::Cancel) && gameState == GameState::PLAYING) {
	//	SetGameState(GameState::PAUSED);
	//
	//} else if (Input::GetMappedDown(Input::Cancel) && gameState == GameState::PAUSED) {
		SetGameState(GameState::MAIN_MENU);
	//
	//// Unpause on ENTER
	//} else if (Input::GetMappedDown(Input::Select) && gameState == GameState::PAUSED) {
	//	SetGameState(GameState::PLAYING);
	}
	//
	//if (GetGameState() != GameState::PLAYING) {
	//	return;
	//}

	halflifePointManager->Update();

	input->Update();
	uiManager->Update();
	entityManager->Update();
	physicsManager->Update();
	audioManager->Update();
	entityManager->LateUpdate();

	hubManager->Update();

	if(Input::GetKeyDown(SDLK_F2)) {
		Input::doCapture = !Input::doCapture;
	}

	// TODO: Move this
	if (gameOver == true) {
		auto diff = AppTime::appTime - gameOverNotifyTime;
		auto fadeOutDuration = 5.0f;
		auto creditsThanksStartTime = 6.0f;
		auto creditsThanksFadeDuration = 3.0f;
		auto creditsNamesStartTime = 8.5f;
		auto creditsNamesFadeDuration = 3.0f;
		auto endGameTime = 10.0f;

		black->SetColour(1.0f, 1.0f, 1.0f, clamp((diff - fadeOutDuration) / fadeOutDuration, 0.0f, 1.0f));
		creditsThanks->SetColour(1.0f, 1.0f, 1.0f, clamp(((diff - creditsThanksStartTime) - creditsThanksFadeDuration) / creditsThanksFadeDuration, 0.0f, 1.0f));
		creditsNames->SetColour(1.0f, 1.0f, 1.0f, clamp(((diff - creditsNamesStartTime) - creditsNamesFadeDuration) / creditsNamesFadeDuration, 0.0f, 1.0f));

		if (diff >= creditsNamesStartTime + creditsNamesFadeDuration + endGameTime) {
			SetGameState(GameState::MAIN_MENU);
		}
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
	camera->ogreCamera->setFOVy(Ogre::Radian(Ogre::Degree(fovDegrees).valueRadians()));

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
	SetFogColor(Ogre::ColourValue{env.fogColor[0], env.fogColor[1], env.fogColor[2]});
	SetSkyColor(Ogre::ColourValue{env.skyColor[0], env.skyColor[1], env.skyColor[2]});
	SetFogDensity(env.fogDensity);

	// TODO: Lerp this, maybe
	sceneManager->setAmbientLight(Ogre::ColourValue{env.ambientColor[0], env.ambientColor[1], env.ambientColor[2]});

	portalManager->SetPortalColor(skyColor);

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
		// mat->setShadingMode(Ogre::SO_PHONG);

		auto pass = mat->getTechnique(0)->getPass(0);
		pass->setAmbient(pass->getDiffuse());

		matIt.moveNext();
	}

	cursor = uiManager->CreateObject<UiImage>("Cursor");
	cursor->SetImage("cursor.png");

	if(nLevel == "hub")
		HubManager::GetSingleton()->NotifyHubLoad();

	if(nLevel == "end") {
		auto endTrigger = entityManager->FindEntity("EndTrigger");
		if(!endTrigger) throw "End missing EndTrigger";

		endTrigger->AddComponent<EndTriggerComponent>();
	}

	auto lend = ck::now();
	auto diff = std::chrono::duration_cast<std::chrono::duration<f32>> (lend-lbegin).count();
	std::cout << "Scene load for " << nLevel << " took " << diff << "s" << std::endl;
}

/*
	                                                                                            
	88888888888                      88   ,ad8888ba,                                            
	88                               88  d8"'    `"8b                                           
	88                               88 d8'                                                     
	88aaaaa     8b,dPPYba,   ,adPPYb,88 88            ,adPPYYba, 88,dPYba,,adPYba,   ,adPPYba,  
	88"""""     88P'   `"8a a8"    `Y88 88      88888 ""     `Y8 88P'   "88"    "8a a8P_____88  
	88          88       88 8b       88 Y8,        88 ,adPPPPP88 88      88      88 8PP"""""""  
	88          88       88 "8a,   ,d88  Y8a.    .a88 88,    ,88 88      88      88 "8b,   ,aa  
	88888888888 88       88  `"8bbdP"Y8   `"Y88888P"  `"8bbdP"Y8 88      88      88  `"Ybbd8"'  
	                                                                                            
	                                                                                            
*/
void App::NotifyEndGame() {
	std::cout << "endgame\n";
	black = uiManager->CreateObject<UiImage>("Black");
	black->SetImage("black.png");
	black->SetAlignment(UiObject::Alignment::Center);
	black->SetPosition(0.0f, 0.0f);
	black->SetSize(GetWindowWidth() * 2, GetWindowHeight() * 2);
	black->SetColour(1.0f, 1.0f, 1.0f, 0.0f);

	creditsThanks = uiManager->CreateObject<UiImage>("CreditsThanks");
	creditsThanks->SetImage("creditsthanks.png");
	creditsThanks->SetAlignment(UiObject::Alignment::BottomCenter);
	creditsThanks->SetPosition(0.0f, 0.3f);
	creditsThanks->SetColour(1.0f, 1.0f, 1.0f, 0.0f);
	creditsThanks->FixedSize(false);

	creditsNames = uiManager->CreateObject<UiImage>("CreditsNames");
	creditsNames->SetImage("creditsnames.png");
	creditsNames->SetAlignment(UiObject::Alignment::TopCenter);
	creditsNames->SetPosition(0.0f, 0.0f);
	creditsNames->SetColour(1.0f, 1.0f, 1.0f, 0.0f);
	creditsNames->FixedSize(false);

	gameOverNotifyTime = AppTime::appTime;
	gameOver = true;
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

	gameOver = false;
	// Destroy all existing UI Objects
	uiManager->DestroyAllObjects();

	// Clear scene completely
	sceneManager->clearScene();

	// Destroy particle systems
	sceneManager->destroyAllParticleSystems();

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