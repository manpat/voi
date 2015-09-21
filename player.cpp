#include "portalmanager.h"
#include "apptime.h"
#include "player.h"
#include "camera.h"
#include "entity.h"
#include "input.h"
#include "app.h"

// TODO: Remove references to ogre stuff
//	All ogre stuff should be handled by camera wrapper
#include <OGRE/OgreSceneNode.h>

void Player::OnInit() {
	std::cout << "New player" << std::endl;
}

void Player::OnUpdate() {
	auto camera = App::GetSingleton()->camera;
	auto portalManager = App::GetSingleton()->portalManager;

	auto md = Input::GetMouseDelta();

	auto nyaw =  -md.x * 2.0 * M_PI * AppTime::deltaTime * 7.f;
	auto npitch = md.y * 2.0 * M_PI * AppTime::deltaTime * 7.f;
	if(abs(cameraPitch + npitch) < M_PI/4.0f) { // Convoluted clamp
		cameraPitch += npitch;
	}
	cameraYaw += nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->setOrientation(ori);

	f32 boost = 2.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost = 4.f;
	}

	// Move with WASD, based on look direction
	// TODO: Don't use ogreSceneNode directly
	if(Input::GetKey(SDLK_w)){
		entity->ogreSceneNode->translate(-oriYaw.zAxis() * AppTime::deltaTime * boost);
	}else if(Input::GetKey(SDLK_s)){
		entity->ogreSceneNode->translate(oriYaw.zAxis() * AppTime::deltaTime * boost);
	}

	if(Input::GetKey(SDLK_a)){
		entity->ogreSceneNode->translate(-oriYaw.xAxis() * AppTime::deltaTime * boost);
	}else if(Input::GetKey(SDLK_d)){
		entity->ogreSceneNode->translate(oriYaw.xAxis() * AppTime::deltaTime * boost);
	}

	if(Input::GetKeyDown('f')){
		static s32 layer = 0;
		layer = (layer+1)%portalManager->GetNumLayers();
		portalManager->SetLayer(layer);
	}
}