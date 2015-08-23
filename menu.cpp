#include <chrono>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreManualObject.h>

#include "menu.h"
#include "app.h"
#include "input.h"
#include "camera.h"

void Menu::Init(App* app) {
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	if (m_quad == nullptr) {
		m_quad = app->sceneManager->createManualObject("MenuQuad");
		m_quad->begin("MenuMat", Ogre::RenderOperation::OT_TRIANGLE_STRIP);

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
		m_quad->index(0);

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

	m_delta = 0.f;
}

void Menu::Terminate(App* app) {
	app->rootNode->removeChild(m_node);
	Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void Menu::Update(App* app, f32 dt) {
	// Rotate camera
	if (m_delta >= 2 * M_PI) {
		m_delta = 0.f;
	}

	auto nyaw = cos(m_delta) * 0.02f * dt;
	auto npitch = cos(m_delta * 2) * 0.02f * dt;

	app->camera->cameraPitch += npitch;
	app->camera->cameraYaw += nyaw;
	
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(app->camera->cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(app->camera->cameraPitch), oriYaw.xAxis()) * oriYaw;
	app->camera->cameraNode->setOrientation(ori);

	m_delta += 0.001f;

	if (Input::GetKey(SDLK_RETURN)) {
		//App::GetSingleton()->gameState = App::GameState::PLAYING;
		app->SetGameState(App::GameState::PLAYING);
	} else if (Input::GetKeyDown(SDLK_ESCAPE)) {
		app->shouldQuit = true;
	}
}