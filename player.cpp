#include "physicsmanager.h"
#include "portalmanager.h"
#include "entitymanager.h"
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
#include <OGRE/OgreCamera.h>

struct PortalTrigger : Component {
	PortalTrigger() : Component(this) {}

	void OnTriggerEnter(ColliderComponent* o) override {
		if(auto portal = o->entity->FindComponent<Portal>()){
			auto targetLayer = (portal->layer[0] == entity->layer) ? portal->layer[1] : portal->layer[0];

			entity->parent->SetLayer(targetLayer);
			portal->shouldDraw = false;
		}
	}
	void OnTriggerLeave(ColliderComponent* o) override {
		if(auto p2 = o->entity->FindComponent<Portal>()){
			p2->shouldDraw = true;
		}
	}

	void OnLayerChange() override {
		auto portalManager = App::GetSingleton()->portalManager;
		portalManager->SetLayer(entity->layer);
	}
};

void Player::OnInit() {
	auto portalTriggerEnt = EntityManager::GetSingleton()->CreateEntity("PortalTrigger", entity->GetPosition());
	portalTrigger = portalTriggerEnt->AddComponent<PortalTrigger>();

	auto portalTriggerCol = portalTriggerEnt->AddComponent<SphereColliderComponent>(vec3{0.5f}, true);
	portalTriggerCol->SetTrigger(true);
	portalTriggerCol->SetKinematic(true);
	portalTriggerCol->SetAutosleep(false);

	entity->AddChild(portalTriggerEnt);
}

void Player::OnAwake() {
	if(!entity->collider) throw "Player requires a collider component";

	// TODO: Abstract setFriction
	entity->collider->body->setFriction(0);
	entity->collider->SetAutosleep(false);
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

	// TODO: Move elsewhere
	f32 boost = 8.f;
	f32 jumpImpulse = 10.f;
	f32 interactDistance = 4.f;

	if(Input::GetMapped(Input::Boost)){
		boost *= 1.5f;
	}

	// Move with WASD, based on look direction
	auto velocity = entity->collider->GetVelocity();
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
		velocity += vec3::UNIT_Y*jumpImpulse;
	}

	entity->collider->SetVelocity(velocity);

	if(Input::GetKeyDown('f')){
		entity->SetLayer((entity->layer+1)%portalManager->GetNumLayers());
	}

	// HACK
	if(entity->collider->GetPosition().y < -50.f){
		entity->collider->SetPosition({0.f, 2.f, 0.f});
		entity->collider->SetVelocity({0,0,0});
		entity->SetLayer(0);
	}

	auto physman = PhysicsManager::GetSingleton();

	// Interact
	if(Input::GetMappedDown(Input::Interact)){
		auto rayres = physman->Raycast(
			camera->cameraNode->_getDerivedPosition()-ori.zAxis()*0.45f,
			-ori.zAxis()*interactDistance,
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
	portalTrigger->entity->SetLayer(entity->layer);
}