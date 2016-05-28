#include "voi.h"
#include "input.h"

void UpdatePlayer(Entity*, f32);

void InitEntity(Entity* e) {
	if(!e) return;

	if(e->entityType == Entity::TypePlayer) {
		auto pl = &e->player;
		pl->slopeSpeedAdjustSmooth = 1.f;
		pl->slopeJumpAdjustSmooth = 1.f;

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
}

void EntityOnCollisionEnter(Entity* e0, Entity* e1) {
	if(e0->entityType == Entity::TypePlayer) {
		printf("Entity %.*s collided with %.*s\n", 
			(u32)e0->nameLength, e0->name,
			(u32)e1->nameLength, e1->name);
		fflush(stdout);
	}
}

void EntityOnCollisionLeave(Entity* e0, Entity* e1) {
	if(e0->entityType == Entity::TypePlayer) {
		printf("Entity %.*s stopped colliding with %.*s\n", 
			(u32)e0->nameLength, e0->name,
			(u32)e1->nameLength, e1->name);
		fflush(stdout);
	}
}

void EntityOnTriggerEnter(Entity*, Entity*) {

}

void EntityOnTriggerLeave(Entity*, Entity*) {

}

extern bool debugDrawEnabled;
extern u8 interactiveHover;

constexpr f32 groundDistTolerance = 0.75f;
constexpr f32 playerJumpTimeout = 250.f/1000.f;

void UpdatePlayer(Entity* ent, f32 dt) {
	auto pl = &ent->player;

	auto scn = ent->scene;
	auto feetPos = ent->position - vec3{0, ent->extents.y-0.02f, 0};
	auto groundHit = Raycast(scn, feetPos, vec3{0,-1,0}, 3.f, ent->layers);
	f32 gndHitUpDot = glm::dot(groundHit.hitNormal, vec3{0,1,0});

	f32 mspd = GetFloatOption("input.mousespeed");

	pl->mouseRot += Input::GetMouseDelta() * mspd;
	pl->mouseRot.y = glm::clamp<f32>(pl->mouseRot.y, -PI/2.f, PI/2.f);

	ent->rotation = glm::angleAxis(-pl->mouseRot.x, vec3{0,1,0});

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
	vel *= speed * pl->slopeSpeedAdjustSmooth;

	vel.y = GetEntityVelocity(ent).y;

	pl->jumpTimeout -= dt;
	if(pl->jumpTimeout < 0.f
	&& gndHitUpDot > 0.707f // At least 45° slope
	&& groundHit.distance < groundDistTolerance) {
		pl->canJump = true;
	}

	if(pl->canJump && Input::GetMappedDown(Input::Jump)) {
		// TODO: Math to figure out how high this is
		vel.y = glm::sqrt(30.f)*2.f*pl->slopeJumpAdjustSmooth;
		pl->canJump = false;
		pl->jumpTimeout = playerJumpTimeout;
	}

	SetEntityVelocity(ent, vel);

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

	auto eye = ent->position + pl->eyeOffset;
	auto eyeFwd = ent->rotation * glm::angleAxis(pl->mouseRot.y, vec3{1,0,0}) * vec3{0,0,-1};
	auto eyeHit = Raycast(scn, eye, eyeFwd, 5.f, ent->layers);

	bool interactive = eyeHit.entity && (eyeHit.entity->flags & Entity::FlagInteractive);
	interactiveHover = interactive?1:0;

	if(interactive && Input::GetMappedDown(Input::Interact)) {
		// TODO: Frob thing when frobbing becomes a thing
		fprintf(stderr, "Frob\n");
	}
}
