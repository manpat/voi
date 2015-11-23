#include <chrono>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreEntity.h>

#include "app.h"
#include "menu.h"
#include "input.h"
#include "entity.h"
#include "camera.h"
#include "uiimage.h"
#include "uimanager.h"
#include "entitymanager.h"
#include "portalmanager.h"
#include "mirrormanager.h"
#include "blendersceneloader.h"
#include "layerrenderingmanager.h"

Menu& Menu::Inst() {
	static Menu inst = Menu{};
	return inst;
}

void Menu::Init(App* app) {
	std::cout << "Menu Init" << std::endl;

	app->layerRenderingManager = std::make_shared<LayerRenderingManager>();
	app->portalManager = std::make_shared<PortalManager>();
	app->mirrorManager = std::make_shared<MirrorManager>();
	//app->bellManager = std::make_shared<BellManager>();
	
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/UI", "FileSystem");

	title = app->uiManager->CreateObject<UiImage>("Title");
	title->SetImage("title.png");
	title->SetPosition(-0.5f, 0.3f);

	menu = app->uiManager->CreateObject<UiImage>("Menu");
	menu->SetImage("menu.png");
	menu->SetPosition(-0.74f, 0.03f);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/menu", "FileSystem");
	BlenderSceneLoader scnLdr{};
	scnLdr.Load("GameData/Scenes/menu/menu.scene", app);
	auto& env = scnLdr.environment;

	auto cameraEnt = app->entityManager->CreateEntity("Camera");
	app->camera = cameraEnt->AddComponent<Camera>();
	app->layerRenderingManager->SetCamera(app->camera);

	auto cameraPos = vec3(5, 2, 10);
	app->camera->cameraNode->setPosition(cameraPos);

	auto targetPos = cameraPos + vec3(-0.6f, 0., -1);
	app->camera->ogreCamera->lookAt(targetPos);

	app->layerRenderingManager->SetupRenderQueueInvocationSequence(0);

	app->SetFogColor(Ogre::ColourValue{env.fogColor[0], env.fogColor[1], env.fogColor[2]});
	app->SetSkyColor(Ogre::ColourValue{env.skyColor[0], env.skyColor[1], env.skyColor[2]});
	app->SetFogDensity(env.fogDensity);

	//app->portalManager->SetPortalColor(Ogre::ColourValue(0.366f, 0.491f, 0.515f));

	m_delta = 0.f;
}

void Menu::Terminate(App* app) {
	app->Terminate();
}

void Menu::Update(App* app, f32 dt) {
	// Rotate camera
	if (m_delta >= 2 * M_PI) {
		m_delta = 0.f;
	}

	auto nyaw = cos(m_delta) * 0.02f;
	auto npitch = cos(m_delta * 2) * 0.02f;

	auto oriYaw = Ogre::Quaternion(Ogre::Radian(nyaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(npitch), oriYaw.xAxis()) * oriYaw;
	app->camera->cameraNode->setOrientation(ori);

	m_delta += dt * 0.25f;

	if (Input::GetMappedDown(Input::Select)) {
		app->SetGameState(App::GameState::PLAYING);
	} else if (Input::GetMappedDown(Input::Cancel)) {
		app->shouldQuit = true;
	}
}