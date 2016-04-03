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
#include "hubmanager.h"
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

	title = app->uiManager->CreateObject<UiImage>("MenuTitle");
	title->SetImage("title.png");
	title->SetAlignment(UiObject::Alignment::BottomLeft);
	title->SetPosition(-0.9f, 0.15f);
	title->FixedSize(false);

	start = app->uiManager->CreateObject<UiImage>("MenuStart");
	if (app->hubManager->lastLevelCompleted < 0) {
		start->SetImage("enterstart.png");
	}
	else {
		start->SetImage("entercontinue.png");
	}
	start->SetAlignment(UiObject::Alignment::BottomLeft);
	start->SetPosition(-0.9f, 0.01f);
	start->FixedSize(false);

	quit = app->uiManager->CreateObject<UiImage>("MenuQuit");
	quit->SetImage("escapequit.png");
	quit->SetAlignment(UiObject::Alignment::TopLeft);
	quit->SetPosition(-0.9f, 0.0f);
	quit->FixedSize(false);

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

	auto ww = app->GetWindowWidth();
	auto wh = app->GetWindowHeight();

	// Set FOV based on ratio to maintain xFOV. * 80 equates to 45 degree yFOV at at 16:9 ratio
	auto fov = ((f32)wh / ww) * 80;

	app->camera->ogreCamera->setFOVy(Ogre::Radian(Ogre::Degree(fov > 45 ? 45 : fov).valueRadians()));

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