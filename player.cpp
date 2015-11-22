#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "portalmanager.h"
#include "entitymanager.h"
#include "movableobject.h"
#include "interactable.h"
#include "checkpoint.h"
#include "apptime.h"
#include "player.h"
#include "camera.h"
#include "entity.h"
#include "input.h"
#include "app.h"

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
		auto layerRenderingManager = App::GetSingleton()->layerRenderingManager;
		layerRenderingManager->SetupRenderQueueInvocationSequence(entity->layer);
	}
};

void Player::OnInit() {
	auto portalTriggerEnt = EntityManager::GetSingleton()->CreateEntity("PortalTrigger");
	portalTrigger = portalTriggerEnt->AddComponent<PortalTrigger>();

	auto portalTriggerCol = portalTriggerEnt->AddComponent<SphereColliderComponent>(vec3{0.5f}, true);
	portalTriggerCol->SetTrigger(true);
	portalTriggerCol->SetKinematic(true);
	portalTriggerCol->SetAutosleep(false);

	entity->AddChild(portalTriggerEnt);
}

void Player::OnAwake() {
	if(!entity->collider) throw "Player requires a collider component";

	entity->collider->SetFriction(0);
	entity->collider->SetAutosleep(false);
}

void Player::OnUpdate() {
	auto layerRenderingManager = App::GetSingleton()->layerRenderingManager;
	auto physicsManager = PhysicsManager::GetSingleton();
	auto camera = App::GetSingleton()->camera;
	auto cameraEnt = camera->entity;

	auto md = Input::GetMouseDelta();

	// TODO: the 7.f here is sensitivity. Probably make a setting
	auto nyaw =  -md.x * 2.0 * PI * AppTime::deltaTime * 7.f;
	auto npitch = md.y * 2.0 * PI * AppTime::deltaTime * 7.f;
	const f32 limit = (f32)PI/2.f;

	cameraPitch = (f32)clamp(cameraPitch + npitch, -limit, limit);
	cameraYaw += (f32)nyaw;

	auto oriYaw = quat(Ogre::Radian(cameraYaw), vec3::UNIT_Y);
	auto ori = quat(Ogre::Radian(cameraPitch), oriYaw.xAxis()) * oriYaw;

	cameraEnt->SetGlobalOrientation(ori);

	// TODO: Move elsewhere
	f32 boost = 8.f;
	f32 jumpImpulse = 10.f;
	f32 interactDistance = 4.f;
	f32 pickupDistance = 4.f;

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

	static bool canInfinijump = false;

	// Cheat
	if(Input::GetKeyDown(SDLK_F12) || Input::GetKeyDown(SDLK_BACKSPACE)){
		canInfinijump = !canInfinijump;
		std::cout << "Infinijump: " << (canInfinijump?"enabled":"disabled") << std::endl;
	}

	if(Input::GetMappedDown(Input::Jump)){
		auto rayres = physicsManager->Raycast(
			entity->GetGlobalPosition(),
			-entity->GetUp() * 2.2f,
			entity->layer);

		if(rayres) isJumping = false;

		if(!isJumping || canInfinijump) {
			velocity += vec3::UNIT_Y*jumpImpulse;
			isJumping = true;
		}
	}

	entity->collider->SetVelocity(velocity);

	if(Input::GetKeyDown('f')){
		entity->SetLayer((entity->layer+1)%layerRenderingManager->GetNumLayers());
	}

	if(entity->collider->GetPosition().y < -50.f){
		Respawn();
	}

	if(heldObject) {
		heldObject->entity->SetGlobalPosition(
			cameraEnt->GetGlobalPosition() + cameraEnt->GetForward()*pickupDistance);
	}

	// Interact
	if(Input::GetMappedDown(Input::Interact)){
		if(heldObject) {
			heldObject->NotifyDrop();
			heldObject = nullptr;
			return;
		}

		auto rayres = physicsManager->Raycast(
			cameraEnt->GetGlobalPosition() + cameraEnt->GetForward()*0.25f,
			cameraEnt->GetForward() * interactDistance,
			entity->layer);

		if(rayres){
			auto ent = rayres.collider->entity;

			if(auto interactable = ent->FindComponent<Interactable>()) {
				interactable->Activate();
				//shakeCount = (shakeCount < 0 ? 1 : shakeCount + 1);
				//std::cout << "activ8: " << shakeCount << std::endl;

			}else if(auto movable = ent->FindComponent<Movable>()) {
				heldObject = movable;
				heldObject->NotifyPickup();
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

	if (shakeCount != -1) {
		if (shakeCount <= 0) {
			ShakeCamera(false);
			shakeCount = -1;
		} else {
			ShakeCamera(true, 0.1f);
		}

		std::cout << shakeCount << std::endl;
	}
}

void Player::OnLayerChange(){
	portalTrigger->entity->SetLayer(entity->layer);
}

void Player::Respawn() {
	auto app = App::GetSingleton();
	auto cp = app->currentCheckpoint;

	if (cp) {
		Respawn(cp->playerSpawnPosition, cp->entity->layer);
		SetToOrientation(cp->playerSpawnOrientation);
	} else {
		Respawn(app->playerSpawnPosition, 0);
		SetToOrientation(app->playerSpawnOrientation);
	}
}

void Player::Respawn(vec3 pos, s32 layer) {
	entity->collider->SetPosition(pos);
	entity->collider->SetVelocity({ 0,0,0 });
	entity->SetLayer(layer);
}

void Player::SetToOrientation(const quat& orientation) {
	cameraPitch = orientation.getPitch().valueRadians();
	cameraYaw = orientation.getYaw().valueRadians();

	App::GetSingleton()->camera->entity->SetGlobalOrientation(orientation);
}

void Player::ShakeCamera(bool shake, f32 amount) {
	auto camera = App::GetSingleton()->camera->entity;

	if (shake && amount > 0) {
		auto shakeAmount = vec3(
			(rand() % 1000) / 1000.0f * amount,
			(rand() % 1000) / 1000.0f * amount,
			(rand() % 1000) / 1000.0f * amount
		);

		camera->SetPosition(shakeAmount);
	} else {
		camera->SetPosition(vec3::ZERO);
	}
}