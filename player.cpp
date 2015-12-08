#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "portalmanager.h"
#include "entitymanager.h"
#include "movableobject.h"
#include "interactable.h"
#include "checkpoint.h"
#include "hubmanager.h"
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
			p2->shouldDraw = true;

			if(collidingPortal == p2) {
				auto endSide = collidingPortal->clip.getSide(entity->collider->GetPosition());
				if(endSide == startSide) {
					entity->parent->SetLayer(savedLayer);
				}

				entity->collider->Refilter();
				collidingPortal = nullptr;
			}
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

	static bool canCheat = false;
	static bool canNoClip = false;
	static bool canInfinijump = false;

	if(Input::GetKeyDown(SDLK_DELETE)) {
		canCheat = !canCheat;
	}

	if(canCheat){
		if(Input::GetKeyDown(SDLK_F12) || Input::GetKeyDown(SDLK_BACKSPACE)){
			canInfinijump = !canInfinijump;
			std::cout << "Infinijump: " << (canInfinijump?"enabled":"disabled") << std::endl;
		}

		if(Input::GetKeyDown(']')) AppTime::phystimescale += 0.1;
		if(Input::GetKeyDown('[')) AppTime::phystimescale -= 0.1;

		if(Input::GetKeyDown('f')){
			entity->SetLayer((entity->layer+1)%layerRenderingManager->GetNumLayers());
		}

		if(Input::GetKeyDown('n')){
			canNoClip = !canNoClip;
			entity->collider->SetKinematic(canNoClip);
			std::cout << "NoClip: " << (canNoClip?"enabled":"disabled") << std::endl;
		}

		if(Input::GetKeyDown(SDLK_F5)) {
			app->hubManager->NotifyReturnToHub();
			app->hubManager->NotifyHubLoad();
		}
	}

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
	auto moveOri = canNoClip?ori:oriYaw;

	cameraEnt->SetGlobalOrientation(ori);

	auto velocity = canNoClip?vec3::ZERO:entity->collider->GetVelocity();
	auto velocityScalar = velocity.length();

	velocity.x = 0.;
	velocity.z = 0.;

	// TODO: Move elsewhere
	f32 boost = 8.f;
	f32 jumpImpulse = 10.f;
	f32 interactDistance = 4.f;
	f32 pickupDistance = 4.f;

	// Head bobbing params
	f32 bobAcceleration = 0.1f; // Transition rate
	f32 bobHoriz = 0.04f; // Horizontal bobbing movement
	f32 bobVert = 0.15f; // Vertical bobbing movement
	f32 bobSpeed = velocityScalar * 0.8f;

	if(Input::GetMapped(Input::Boost)){
		boost *= 1.5f;
	}

	// Check if grounded
	isGrounded = physicsManager->Raycast(
		entity->GetGlobalPosition(),
		-entity->GetUp() * 1.505f,
		entity->layer).hit();

	// Head bobbing logic
	if (bobDelta >= 2 * M_PI) {
		bobDelta = 0.0f;
	}

	bobDelta += AppTime::deltaTime * bobSpeed;

	if (isGrounded && !isJumping && velocityScalar > 0.1f) {
		bobPower = bobPower < 1 ? bobPower + bobAcceleration : 1.0f;
	} else {
		bobPower = bobPower > 0 ? bobPower - bobAcceleration : 0.0f;
	}

	cameraOffset.x += cos(bobDelta) * bobPower * bobHoriz;
	cameraOffset.y += (std::abs(cos(bobDelta)) - .5f) * bobPower * bobVert;

	// Movement logic
	if(Input::GetMapped(Input::Forward)){
		velocity -= moveOri.zAxis() * boost;
	}else if(Input::GetMapped(Input::Backward)){
		velocity += moveOri.zAxis() * boost;
	}

	if(Input::GetMapped(Input::Left)){
		velocity -= moveOri.xAxis() * boost;
	}else if(Input::GetMapped(Input::Right)){
		velocity += moveOri.xAxis() * boost;
	}

	if (isJumping && isGrounded) {
		isJumping = false;
	}

	if (Input::GetMappedDown(Input::Jump)) {
		if (!isJumping || canInfinijump) {
			velocity += vec3::UNIT_Y * jumpImpulse;
			isJumping = true;
		}
	}

	if(canNoClip){
		auto pos = entity->GetPosition();
		entity->SetPosition(pos + velocity * AppTime::deltaTime);
	}else{
		entity->collider->SetVelocity(velocity);
	}

	if(entity->collider->GetPosition().y < -50.f){
		Respawn();
	}

	// if(heldObject) {
	// 	heldObject->entity->SetGlobalPosition(
	// 		cameraEnt->GetGlobalPosition() + cameraEnt->GetForward()*pickupDistance);
	// }

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

	camera->entity->SetPosition(localCameraPosition + cameraOffset);

	cameraOffset = vec3::ZERO;
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

	auto shakeAmount = vec3::ZERO;

	if (amount > 0) {
		shakeAmount.x = (rand() % 1000) / 1000.0f * amount;
		shakeAmount.y = (rand() % 1000) / 1000.0f * amount;
		shakeAmount.z = (rand() % 1000) / 1000.0f * amount;

		maxShake = amount;
	}

	cameraOffset += shakeAmount;
}