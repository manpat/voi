#include "portalmanager.h"
#include "physicsmanager.h"
#include "apptime.h"
#include "player.h"
#include "camera.h"
#include "entity.h"
#include "input.h"
#include "app.h"

// TODO: Remove references to ogre stuff
//	All ogre stuff should be handled by camera wrapper
#include <OGRE/OgreSceneNode.h>

void Player::OnAwake() {
	std::cout << "New player" << std::endl;
	collider = entity->FindComponent<ColliderComponent>();
	if(!collider) throw "Player requires a collider component";
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
	camera->cameraNode->_setDerivedOrientation(ori);

	f32 boost = 2.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost = 6.f;
	}

	// Move with WASD, based on look direction
	auto velocity = collider->velocity;
	velocity.x = 0.;
	velocity.z = 0.;

	if(Input::GetKey(SDLK_w)){
		velocity -= oriYaw.zAxis() * boost;
	}else if(Input::GetKey(SDLK_s)){
		velocity += oriYaw.zAxis() * boost;
	}

	if(Input::GetKey(SDLK_a)){
		velocity -= oriYaw.xAxis() * boost;
	}else if(Input::GetKey(SDLK_d)){
		velocity += oriYaw.xAxis() * boost;
	}

	collider->velocity = velocity;

	if(Input::GetKeyDown('f')){
		static s32 layer = 0;
		layer = (layer+1)%portalManager->GetNumLayers();
		portalManager->SetLayer(layer);
	}
}