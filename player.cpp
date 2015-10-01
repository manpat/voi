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

	collider->SetAutosleep(false);
	collider->collisionGroups = 1<<0; // Layer 1;

	// TODO: Abstract
	collider->body->setFriction(0);
}

void Player::OnUpdate() {
	auto camera = App::GetSingleton()->camera;
	auto portalManager = App::GetSingleton()->portalManager;
	auto physicsManager = App::GetSingleton()->physicsManager;

	auto md = Input::GetMouseDelta();

	// TODO: the 7.f here is sensitivity. Probably make a setting
	auto nyaw =  -md.x * 2.0 * PI * AppTime::deltaTime * 7.f;
	auto npitch = md.y * 2.0 * PI * AppTime::deltaTime * 7.f;
	const f32 limit = PI/2.f;

	cameraPitch = clamp(cameraPitch+npitch, -limit, limit);
	cameraYaw += (f32)nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->_setDerivedOrientation(ori);

	f32 boost = 6.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost *= 1.5f;
	}

	// Move with WASD, based on look direction
	auto velocity = collider->GetVelocity();
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

	if(Input::GetKeyDown(SDLK_SPACE)){
		velocity += vec3::UNIT_Y*10.;
	}

	collider->SetVelocity(velocity);

	if(Input::GetKeyDown('f')){
		static s32 layer = 0;
		layer = (layer+1)%portalManager->GetNumLayers();
		portalManager->SetLayer(layer);
		collider->collisionGroups = 1<<layer;
		collider->Refilter();
	}
}