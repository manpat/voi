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

#define NEW_MENU

Menu& Menu::Inst() {
	static Menu inst = Menu{};
	return inst;
}

void Menu::Init(App* app) {
	std::cout << "Menu Init" << std::endl;

#ifdef NEW_MENU
	Ogre::ColourValue sky(1, 1, 1);
	app->sceneManager->setFog(Ogre::FOG_LINEAR, sky, 0, 15.0f, 30.0f);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Scenes/Menu", "FileSystem");
	BlenderSceneLoader{}.Load("GameData/Scenes/Menu/menu.scene", app);

	auto cameraEnt = app->entityManager->CreateEntity();
	app->camera = cameraEnt->AddComponent<Camera>();

	app->camera->cameraNode->setPosition(0, 1.0, 12.0);
	app->camera->viewport->setBackgroundColour(sky);
#else
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	if (m_quad == nullptr) {
		m_quad = app->sceneManager->createManualObject("MenuQuad");
		m_quad->begin("MenuMat", Ogre::RenderOperation::OT_TRIANGLE_FAN);

		m_quad->position(-1.5, -1.0, 0.0);
		m_quad->textureCoord(0, 1);

		m_quad->position(1.5, -1.0, 0.0);
		m_quad->textureCoord(1, 1);

		m_quad->position(1.5, 1.0, 0.0);
		m_quad->textureCoord(1, 0);

		m_quad->position(-1.5, 1.0, 0.0);
		m_quad->textureCoord(0, 0);

		m_quad->index(0);
		m_quad->index(1);
		m_quad->index(2);
		m_quad->index(3);

		m_quad->end();
	}

	if (m_node == nullptr) {
		m_node = app->rootNode->createChildSceneNode();
		m_node->attachObject(m_quad);
	} else {
		app->rootNode->addChild(m_node);
	}

	app->camera->cameraNode->setPosition(0, 0, 3.0);
	app->camera->viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
#endif

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

	// TODO: Use input mapping interface
	if (Input::GetKeyDown(SDLK_RETURN)) {
		app->SetGameState(App::GameState::PLAYING);
	} else if (Input::GetKeyDown(SDLK_ESCAPE)) {
		app->shouldQuit = true;
	}
}