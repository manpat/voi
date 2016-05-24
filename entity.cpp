#include "voi.h"
#include "input.h"

void UpdatePlayer(Entity*, f32);

void UpdateEntity(Entity* e, f32 dt) {
	if(!e) return;

	switch(e->entityType) {
		case Entity::TypePlayer: UpdatePlayer(e, dt); break;
		default: break;
	}
}

void EntityOnCollisionEnter(Entity* e0, Entity* e1) {
	printf("Entity %.*s collided with %.*s\n", 
		(u32)e0->nameLength, e0->name,
		(u32)e1->nameLength, e1->name);
}

void EntityOnCollisionLeave(Entity*, Entity*) {

}

void EntityOnTriggerEnter(Entity*, Entity*) {

}

void EntityOnTriggerLeave(Entity*, Entity*) {

}

extern bool debugDrawEnabled;
extern u8 interactiveHover;

void UpdatePlayer(Entity* pl, f32) {
	auto scn = pl->scene;
	
	auto& mouseRot = pl->player.mouseRot;
	mouseRot += Input::GetMouseDelta();
	mouseRot.y = glm::clamp<f32>(mouseRot.y, -PI/2.f, PI/2.f);

	pl->rotation = glm::angleAxis(-mouseRot.x, vec3{0,1,0});
	SetEntityRotation(pl, pl->rotation);

	vec3 vel {};
	if(Input::GetMapped(Input::Forward))	vel += pl->rotation * vec3{0,0,-1};
	if(Input::GetMapped(Input::Backward))	vel += pl->rotation * vec3{0,0, 1};
	if(Input::GetMapped(Input::Left))		vel += pl->rotation * vec3{-1,0,0};
	if(Input::GetMapped(Input::Right))		vel += pl->rotation * vec3{ 1,0,0};

	if(glm::length(vel) > 1.f){
		vel = glm::normalize(vel);
	}

	f32 speed = 8.f;
	if(Input::GetMapped(Input::Boost)) speed *= 2.f;
	vel *= speed;

	vel.y = GetEntityVelocity(pl).y;

	if(Input::GetMappedDown(Input::Jump)) {
		vel.y += glm::sqrt(30.f)*2.f;
	}

	SetEntityVelocity(pl, vel);

	if(Input::GetKeyDown('1')) { pl->layers = 1<<0; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('2')) { pl->layers = 1<<1; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('3')) { pl->layers = 1<<2; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('4')) { pl->layers = 1<<3; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('5')) { pl->layers = 1<<4; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('6')) { pl->layers = 1<<5; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('7')) { pl->layers = 1<<6; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('8')) { pl->layers = 1<<7; scn->physicsContext.needsRefilter = true; }
	if(Input::GetKeyDown('9')) { pl->layers = 1<<8; scn->physicsContext.needsRefilter = true; }

	if(Input::GetKeyDown('c')) Input::doCapture ^= true;
	if(Input::GetKeyDown(SDLK_F1)) debugDrawEnabled ^= true;

	auto eye = pl->position + pl->player.eyeOffset;
	auto eyeFwd = pl->rotation * glm::angleAxis(mouseRot.y, vec3{1,0,0}) * vec3{0,0,-1};
	auto hit = Raycast(scn, eye, eyeFwd, 5.f, pl->layers);

	bool interactive = hit.entity && (hit.entity->flags & Entity::FlagInteractive);
	interactiveHover = interactive?1:0;

	if(interactive && Input::GetMappedDown(Input::Interact)) {
		// TODO: Frob thing when frobbing becomes a thing
	}
}
