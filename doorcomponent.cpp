#include "movercomponent.h"
#include "physicsmanager.h"
#include "doorcomponent.h"
#include "entity.h"

void DoorComponent::OnAwake() {
	mover = entity->AddComponent<MoverComponent>();
	if(!entity->collider) throw "DoorComponent requires collider";

	entity->collider->SetKinematic(true);
	entity->collider->SetAutosleep(false);

	closedPosition = entity->collider->GetPosition();
	openPosition = closedPosition + vec3::UNIT_Y*entity->collider->dimensions.y;
	isOpen = false;
}

void DoorComponent::OnMessage(const std::string& msg, const OpaqueType& ot){
	auto trigent = (*ot.Get<Component*>(true))->entity;

	if(msg == "open"){
		switchStates = ~0u;
		UpdateState();
		trigent->SendMessage("dooropen", (Component*)this);

	}else if(msg == "close"){
		switchStates = 0;
		UpdateState();
		trigent->SendMessage("doorclose", (Component*)this);

	}else if(msg.substr(0, 6) == "unlock"){
		auto numStr = msg.substr(6);
		if(numStr.size() == 0){
			std::cout << "Warning!! 'unlock' message sent to door missing number" << std::endl;
			return;
		}

		auto num = std::stol(numStr);
		if(ordered){
			switchStates |= (1<<num) & requiredMask;
			bool po2 = false;

			for(u32 i = 0; i < 30; ++i){
				po2 = ((switchStates+1) == 1u<<i);
				if(po2) break;
			}

			if(!po2) {
				switchStates = 0;
				std::cout << "Invalid combination" << std::endl;
			}

			UpdateState();
			if(!isOpen) {
				trigent->SendMessage((switchStates==0)?"incorrect":"correct", (Component*)this);
			}else{
				trigent->SendMessage("dooropen", (Component*)this);
			}

		}else{
			switchStates |= (1<<num) & requiredMask;
			UpdateState();
			if(!isOpen) {
				trigent->SendMessage("correct", (Component*)this);
			}else{
				trigent->SendMessage("dooropen", (Component*)this);
			}
		}
	}
}

void DoorComponent::UpdateState() {
	// If all required switch states are set
	if((switchStates&requiredMask) == requiredMask){
		// Do the things
		if(!isOpen) mover->MoveTo(openPosition, openTime);
		isOpen = true;

	}else{
		if(isOpen) mover->MoveTo(closedPosition, openTime);
		isOpen = false;
	}
}