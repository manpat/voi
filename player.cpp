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
	Portal* collidingPortal = nullptr;
	u32 startSide = 0;
	s32 savedLayer = -1;

	PortalTrigger() : Component(this) {}

	void OnTriggerEnter(ColliderComponent* o) override {
		if(auto portal = o->entity->FindComponent<Portal>()){
			// std::cout << "[x] OnTriggerEnter " << entity->layer << "\n";
			if(collidingPortal) return;

			savedLayer = entity->layer;
			auto targetLayer = (portal->layer[0] == savedLayer) ? portal->layer[1] : portal->layer[0];

			entity->parent->SetLayer(targetLayer);
			portal->shouldDraw = false;

			startSide = portal->clip.getSide(entity->collider->GetPosition());
			collidingPortal = portal;
		}
	}
	void OnTriggerLeave(ColliderComponent* o) override {
		if(auto p2 = o->entity->FindComponent<Portal>()){
			// std::cout << "[ ] OnTriggerLeave " << entity->layer << "\n";
			p2->shouldDraw = true;

			if(collidingPortal == p2) {
				auto endSide = collidingPortal->clip.getSide(entity->collider->GetPosition());
				if(endSide == startSide) {
					// std::cout << "Same side!" << std::endl;
					entity->parent->SetLayer(savedLayer);
				}

				entity->collider->Refilter();
				collidingPortal = nullptr;
			}

			// std::cout << std::endl;
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

	auto portalTriggerCol = portalTriggerEnt->AddComponent<SphereColliderComponent>(vec3{0.7f}, true);
	portalTriggerCol->SetTrigger(true);
	portalTriggerCol->SetKinematic(true);
	portalTriggerCol->SetAutosleep(false);
	portalTriggerCol->SetContinuous(true);

	entity->AddChild(portalTriggerEnt);
	localCameraPosition = App::GetSingleton()->camera->entity->GetPosition();
}

void Player::OnAwake() {
	if(!entity->collider) throw "Player requires a collider component";

	entity->collider->SetFriction(0);
	entity->collider->SetAutosleep(false);
}

void Player::OnUpdate() {
	auto physicsManager = PhysicsManager::GetSingleton();
	auto app = App::GetSingleton();

	auto layerRenderingManager = app->layerRenderingManager;
	auto hSens = (f32)app->hMouseSensitivity * app->GetWindowWidth() / 10000.f;
	auto vSens = (f32)app->vMouseSensitivity * app->GetWindowHeight() / 10000.f;

	auto camera = app->camera;
	auto cameraEnt = camera->entity;

	auto md = Input::GetMouseDelta();

	// TODO: the 7.f here is sensitivity. Probably make a setting
	auto nyaw =  -md.x * 2.0 * PI * AppTime::deltaTime * hSens;
	auto npitch = md.y * 2.0 * PI * AppTime::deltaTime * vSens;
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

	if(Input::GetKeyDown(']')) AppTime::phystimescale += 0.1;
	if(Input::GetKeyDown('[')) AppTime::phystimescale -= 0.1;

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

	maxShake = 0.0f;
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

void Player::ShakeCamera(f32 amount) {
	if (maxShake >= amount) {
		return;
	}

	auto camera = App::GetSingleton()->camera->entity;
	auto shakeAmount = vec3::ZERO;

	if (amount > 0) {
		shakeAmount.x = (rand() % 1000) / 1000.0f * amount;
		shakeAmount.y = (rand() % 1000) / 1000.0f * amount;
		shakeAmount.z = (rand() % 1000) / 1000.0f * amount;

		maxShake = amount;
	}

	camera->SetPosition(localCameraPosition + shakeAmount);
}