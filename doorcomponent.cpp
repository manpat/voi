#include "movercomponent.h"
#include "physicsmanager.h"
#include "doorcomponent.h"
#include "entity.h"

void DoorComponent::OnAwake() {
	mover = entity->AddComponent<MoverComponent>();
	collider = entity->FindComponent<ColliderComponent>();
	if(!collider) throw "DoorComponent requires collider";

	collider->SetKinematic(true);
	collider->SetAutosleep(false);

	closedPosition = collider->GetPosition();
	openPosition = closedPosition + vec3::UNIT_Y*collider->dimensions.y;
	isOpen = false;
}

void DoorComponent::OnMessage(const std::string& msg, const OpaqueType&){
	if(msg == "open"){
		isOpen = true;
		mover->MoveTo(openPosition, 2.f);

	}else if(msg == "close"){
		isOpen = false;
		mover->MoveTo(closedPosition, 2.f);

	}else if(msg == "toggle"){
		isOpen = !isOpen;
		mover->MoveTo(isOpen?openPosition:closedPosition, 2.f);
	}
}
