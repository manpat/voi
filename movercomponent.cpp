#include "movercomponent.h"
#include "physicsmanager.h"
#include "apptime.h"
#include "entity.h"
#include "player.h"
#include "app.h"

void MoverComponent::OnUpdate(){
	if(!moving) return;

	vec3 npos;

	if(a > 1.f){
		moving = false;
		npos = fromPosition + positionDiff;
		entity->SendMessage("movecomplete");

		// App::GetSingleton()->player->shakeCount--; // Manpat: I don't like this
	}else{
		npos = fromPosition + positionDiff * a;
		a += (f32)AppTime::scaledDeltaTime / animationLength;
	}

	if(entity->collider && !entity->collider->kinematic){
		entity->collider->SetPosition(npos);
	}else{
		entity->SetPosition(npos);
	}
}

void MoverComponent::MoveTo(const vec3& npos, f32 inTime){
	fromPosition = entity->GetPosition();
	positionDiff = npos - fromPosition;

	animationLength = inTime;
	moving = true;
	a = 0.f;
}