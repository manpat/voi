#include <chrono>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreManualObject.h>

#include "menu.h"
#include "app.h"
#include "input.h"
#include "camera.h"
#include "entity.h"
#include "entitymanager.h"
#include "blendersceneloader.h"

Menu& Menu::Inst() {
	static Menu inst = Menu{};
	return inst;
}

void Menu::Init(App* app) {
	std::cout << "Menu Init" << std::endl;

	Ogre::ColourValue sky(0.484375, 0.73046875, 1);
	app->sceneManager->setFog(Ogre::FOG_LINEAR, sky, 0, 15.0f, 30.0f);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/Menu", "FileSystem");
	BlenderSceneLoader{}.Load("GameData/Scenes/Menu/menu.scene", app);

	auto cameraEnt = app->entityManager->CreateEntity();
	app->camera = cameraEnt->AddComponent<Camera>();

	app->camera->cameraNode->setPosition(0, 1.0, 12.0);
	app->camera->viewport->setBackgroundColour(sky);

	m_delta = 0.f;
}

void Menu::Terminate(App* app) {
	app->ResetScene();
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