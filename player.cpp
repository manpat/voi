#include "physicsmanager.h"
#include "portalmanager.h"
#include "interactable.h"
#include "apptime.h"
#include "player.h"
#include "camera.h"
#include "entity.h"
#include "input.h"
#include "app.h"

// TODO: Remove references to ogre stuff
//	All camera stuff should be handled by camera wrapper
#include <OGRE/OgreSceneNode.h>

void Player::OnAwake() {
	collider = entity->FindComponent<ColliderComponent>();
	if(!collider) throw "Player requires a collider component";

	collider->SetAutosleep(false);
	entity->SetLayer(0);

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
	const f32 limit = (f32)PI/2.f;

	cameraPitch = (f32)clamp(cameraPitch+npitch, -limit, limit);
	cameraYaw += (f32)nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->_setDerivedOrientation(ori);

	f32 boost = 10.f;
	f32 jumpHeight = 10.0;

	if(Input::GetKey(Input::Boost)){
		boost *= 1.5f;
	}

	// Move with WASD, based on look direction
	auto velocity = collider->GetVelocity();
	velocity.x = 0.;
	velocity.z = 0.;

	if(Input::GetMapped(Input::Forward)){
		velocity -= oriYaw.zAxis() * boost;
	}else if(Input::GetMapped(Input::Backward)){
		velocity += oriYaw.zAxis() * boost;
	}

	if(Input::GetMapped(Input::Left)){
		velocity -= oriYaw.xAxis() * boost;
	}else if(Input::GetMapped(Input::Right)){
		velocity += oriYaw.xAxis() * boost;
	}

	if(Input::GetMappedDown(Input::Jump)){
		velocity += vec3::UNIT_Y*jumpHeight;
	}

	collider->SetVelocity(velocity);

	if(Input::GetKeyDown('f')){
		entity->SetLayer((entity->layer+1)%portalManager->GetNumLayers());
	}

	// HACK
	if(collider->GetPosition().y < -50.f){
		collider->SetPosition({0.f, 2.f, 0.f});
		collider->SetVelocity({0,0,0});
		entity->SetLayer(0);
	}

	auto physman = PhysicsManager::GetSingleton();

	// Interact
	if(Input::GetMappedDown(Input::Interact)){
		auto rayres = physman->Raycast(
			camera->cameraNode->_getDerivedPosition()-ori.zAxis()*0.5f,
			-ori.zAxis()*3.f,
			entity->layer);

		if(rayres){
			auto interactable = rayres.collider->entity->FindComponent<Interactable>();

			std::cout
				<< rayres.collider->entity->GetName()
				<< "\tIs interactable? "  
				<< std::boolalpha
				<< (interactable != nullptr)
				<< std::endl;

			if(interactable){
				interactable->Activate();
			}
		}
	}

	// // This prints out what you're looking at
	// auto rayres = PhysicsManager::GetSingleton()->Raycast(
	// 	entity->GetPosition()+vec3::UNIT_Y*0.2f -ori.zAxis()*0.3f, 
	// 	-ori.zAxis()*10.f,
	// 	/*entity->layer*/-1,
	// 	collider->collisionGroups);

	// if(rayres){
	// 	std::cout << rayres.collider->entity->GetName() << std::endl;
	// }
}

void Player::OnLayerChange(){
	auto portalManager = App::GetSingleton()->portalManager;
	
	portalManager->SetLayer(entity->layer);
	collider->collisionGroups = 1<<entity->layer;
	collider->Refilter();
}

void Player::OnTriggerEnter(ColliderComponent* o){
	if(auto portal = o->entity->FindComponent<Portal>()){
		EnterPortal(portal);
	}
}
void Player::OnTriggerLeave(ColliderComponent* o){
	if(auto portal = o->entity->FindComponent<Portal>()){
		LeavePortal(portal);
	}
}

void Player::EnterPortal(Portal* portal){
	if(!portal) return;

	if(portal->layer[0] == entity->layer){
		entity->SetLayer(portal->layer[1]);
	}else{
		entity->SetLayer(portal->layer[0]);
	}
}

void Player::LeavePortal(Portal* portal){
	if(!portal) return;
}