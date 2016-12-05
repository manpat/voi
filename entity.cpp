#include "voi.h"
#include "input.h"

void UpdatePlayer(Entity*, f32);
void UpdatePlayerPhysics(Entity*, f32);

void InitEntity(Entity* e) {
	if(!e) return;

	if(e->entityType == Entity::TypePlayer) {
		auto pl = &e->player;
		pl->slopeSpeedAdjustSmooth = 1.f;
		pl->slopeJumpAdjustSmooth = 1.f;
		pl->collidingPortalID = 0;

		ConstrainEntityUpright(e);
	} 
}

void DeinitEntity(Entity*) {

}

void UpdateEntity(Entity* e, f32 dt) {
	if(!e) return;

	switch(e->entityType) {
		case Entity::TypePlayer: UpdatePlayer(e, dt); break;
		default: break;
	}

	if(e->initCallback) {
		RunCallback(e->id, e->initCallback);
		e->initCallback = 0;
		// NOTE: I'm not sure I like modifying this but I see no reason
		//	to keep it around
	}

	if(e->updateCallback) {
		RunCallback(e->id, e->updateCallback);
	}
}

void UpdateEntityPhysicsRate(Entity* e, f32 dt) {
	// TODO: Any player physics stuff
}

void EntityOnCollisionEnter(Entity* e0, Entity* e1) {
	if(e0->entityType == Entity::TypePortal && e1->entityType == Entity::TypePlayer) {
		assert(!(e1->layers & (e1->layers-1)) && "Player is on more than one layer!");

		if(e1->player.collidingPortalID) {
			auto ptl = GetEntity(e1->player.collidingPortalID);

			LogError("Warning! Ignoring portal collision because a portal is already being tracked!\n"
				"Portals %.*s and %.*s are probably too close together!\n", 
				ptl->nameLength, ptl->name,
				e0->nameLength, e0->name);
			return;
		}

		e1->player.collidingPortalID = e0->id;
		e1->player.originalLayers = e1->layers;

		auto diff = e1->position - e0->position;
		auto sidef = glm::dot(diff, e0->rotation * e0->planeNormal);
		e1->player.portalSide = (s8)(sidef/glm::abs(sidef));
	}

	if(e0->entityType == Entity::TypeTrigger && e1->entityType == Entity::TypePlayer) {
		if(e0->trigger.enterCallback){
			RunCallback(e0->id, e0->trigger.enterCallback);
		}
	}
}

void EntityOnCollisionLeave(Entity* e0, Entity* e1) {
	if(e0->entityType == Entity::TypePortal && e1->entityType == Entity::TypePlayer) {
		if(e1->player.collidingPortalID == e0->id) {
			e1->player.collidingPortalID = 0;
		}
	}

	if(e0->entityType == Entity::TypeTrigger && e1->entityType == Entity::TypePlayer) {
		if(e0->trigger.leaveCallback){
			RunCallback(e0->id, e0->trigger.leaveCallback);
		}
	}
}

extern bool debugDrawEnabled;
bool flyMode = false;

constexpr f32 groundDistTolerance = 0.75f;
constexpr f32 playerJumpTimeout = 250.f/1000.f;

void UpdatePlayer(Entity* ent, f32 dt) {
	auto pl = &ent->player;

	if(auto ptl = GetEntity(pl->collidingPortalID)) {
		vec3 ptlPos = ptl->position + ptl->rotation * ptl->centerOffset;
		auto diff = ptlPos - ent->position;
		auto sidef = glm::dot(diff, ptl->rotation * ptl->planeNormal)*pl->portalSide;

		if(sidef > 0.f) {
			ent->layers = pl->originalLayers ^ ptl->layers;
		}else{
			ent->layers = pl->originalLayers;
		}
		RefilterEntity(ent);

		pl->camera->intersectingPortalId = pl->collidingPortalID;
	}else{
		pl->camera->intersectingPortalId = 0;
	}

	// Cheat keys
	if(Input::GetKeyDown(SDLK_F2)) {
		flyMode ^= true;

		SetEntityKinematic(ent, flyMode, false /* Don't change activation state */);
	}

	if(Input::GetKeyDown('1')) { ent->layers = 1<<0; RefilterEntity(ent); }
	if(Input::GetKeyDown('2')) { ent->layers = 1<<1; RefilterEntity(ent); }
	if(Input::GetKeyDown('3')) { ent->layers = 1<<2; RefilterEntity(ent); }
	if(Input::GetKeyDown('4')) { ent->layers = 1<<3; RefilterEntity(ent); }
	if(Input::GetKeyDown('5')) { ent->layers = 1<<4; RefilterEntity(ent); }
	if(Input::GetKeyDown('6')) { ent->layers = 1<<5; RefilterEntity(ent); }
	if(Input::GetKeyDown('7')) { ent->layers = 1<<6; RefilterEntity(ent); }
	if(Input::GetKeyDown('8')) { ent->layers = 1<<7; RefilterEntity(ent); }
	if(Input::GetKeyDown('9')) { ent->layers = 1<<8; RefilterEntity(ent); }

	if(Input::GetKeyDown('c')) Input::doCapture ^= true;
	if(Input::GetKeyDown(SDLK_F1)) debugDrawEnabled ^= true;

	auto scn = ent->scene;
	auto feetPos = ent->position - vec3{0, ent->extents.y-0.02f, 0};
	auto groundHit = Raycast(scn, feetPos, vec3{0,-1,0}, 3.f, ent->layers);
	f32 gndHitUpDot = glm::dot(groundHit.hitNormal, vec3{0,1,0});

	f32 mspd = GetFloatOption("input.mousespeed");

	pl->mouseRot += Input::GetMouseDelta() * mspd;
	pl->mouseRot.y = glm::clamp<f32>(pl->mouseRot.y, -PI/2.f, PI/2.f);

	ent->rotation = glm::angleAxis(-pl->mouseRot.x, vec3{0,1,0});

	if(!flyMode) {
		vec3 vel {};
		if(Input::GetMapped(Input::Forward))	vel += ent->rotation * vec3{0,0,-1};
		if(Input::GetMapped(Input::Backward))	vel += ent->rotation * vec3{0,0, 1};
		if(Input::GetMapped(Input::Left))		vel += ent->rotation * vec3{-1,0,0};
		if(Input::GetMapped(Input::Right))		vel += ent->rotation * vec3{ 1,0,0};

		if(glm::length(vel) > 1.f){
			vel = glm::normalize(vel);
		}

		// Slow player when moving up a slope
		auto gndHitFwdDot = glm::dot(vel, groundHit.hitNormal);
		f32 slopeSpeedAdjust = glm::clamp(gndHitUpDot+gndHitFwdDot, 0.7f, 1.f);

		// Only allow speed adjustment to change while on ground
		if(groundHit.distance < groundDistTolerance){
			pl->slopeSpeedAdjustSmooth = glm::mix(pl->slopeSpeedAdjustSmooth, slopeSpeedAdjust, 6.f*dt);
		}

		pl->slopeJumpAdjustSmooth = glm::mix(pl->slopeJumpAdjustSmooth, glm::max(slopeSpeedAdjust, 0.85f), 12.f*dt);

		f32 speed = 8.f;
		if(Input::GetMapped(Input::Boost)) speed *= 2.f;
		if(Input::GetKey(SDLK_LCTRL)) speed *= 0.1f;
		vel *= speed * pl->slopeSpeedAdjustSmooth;

		pl->jumpTimeout -= dt;
		if(pl->jumpTimeout < 0.f
		&& gndHitUpDot > 0.707f // At least 45° slope
		&& groundHit.distance < groundDistTolerance) {
			pl->canJump = true;
		}

		vel.y = GetEntityVelocity(ent).y;

		if(pl->canJump && Input::GetMappedDown(Input::Jump)) {
			// TODO: Math to figure out how high this is
			vel.y = glm::sqrt(30.f)*2.f*pl->slopeJumpAdjustSmooth;
			pl->canJump = false;
			pl->jumpTimeout = playerJumpTimeout;
		}

		SetEntityVelocity(ent, vel);
	}else{
		auto moveRot = ent->rotation * glm::angleAxis(pl->mouseRot.y, vec3{1,0,0});

		vec3 vel {};
		if(Input::GetMapped(Input::Forward))	vel += moveRot * vec3{0,0,-1};
		if(Input::GetMapped(Input::Backward))	vel += moveRot * vec3{0,0, 1};
		if(Input::GetMapped(Input::Left))		vel += moveRot * vec3{-1,0,0};
		if(Input::GetMapped(Input::Right))		vel += moveRot * vec3{ 1,0,0};

		f32 speed = 8.f;
		if(Input::GetMapped(Input::Boost)) speed *= 2.f;
		if(Input::GetKey(SDLK_LCTRL)) speed *= 0.1f;

		ent->position += vel * dt * speed;
	}

	auto eye = ent->position + pl->eyeOffset;
	auto eyeFwd = ent->rotation * glm::angleAxis(pl->mouseRot.y, vec3{1,0,0}) * vec3{0,0,-1};
	auto eyeHit = Raycast(scn, eye, eyeFwd, 5.f, ent->layers);

	pl->lookingAtInteractive = eyeHit.entity && (eyeHit.entity->entityType == Entity::TypeInteractive);
	if(pl->lookingAtInteractive && Input::GetMappedDown(Input::Interact)) {
		auto e = eyeHit.entity;
		// LogError("Frob %.*s\n", e->nameLength, e->name);

		if(e->interact.frobCallback) {
			RunCallback(e->id, e->interact.frobCallback);
		}
	}
}

const char* GetEntityTypeName(u8 type) {
	static const char* names[] = {
		"Geometry",
		"Portal",
		"Mirror",
		"Interactive",
		"Trigger",
	};

	static const char* neNames[] = {
		"Player",
	};

	if(type < Entity::TypeNonExportable && type < sizeof(names)/sizeof(names[0])) {
		return names[type];
	}

	type -= Entity::TypeNonExportable;
	if(type < sizeof(neNames)/sizeof(neNames[0])) {
		return neNames[type];
	}

	return "<unknown>";
}
