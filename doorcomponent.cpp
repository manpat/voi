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

void DoorComponent::OnMessage(const std::string& msg, const OpaqueType&){
	if(msg == "open"){
		switchStates = ~0u;
		UpdateState();

	}else if(msg == "close"){
		switchStates = 0;
		UpdateState();

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

		}else{
			switchStates |= (1<<num) & requiredMask;
			UpdateState();
		}
	}
}

void DoorComponent::UpdateState() {
	// If all required switch states are set 
	if((switchStates&requiredMask) == requiredMask){
		// Do the things
		if(!isOpen) mover->MoveTo(openPosition, 2.f);		
		isOpen = true;
		
	}else{
		if(isOpen) mover->MoveTo(closedPosition, 2.f);
		isOpen = false;
	}
}
