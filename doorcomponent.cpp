#include "movercomponent.h"
#include "physicsmanager.h"
#include "doorcomponent.h"
#include "entity.h"
#include "player.h"
#include "app.h"

void DoorComponent::OnAwake() {
	mover = entity->AddComponent<MoverComponent>();
	if(!entity->collider) throw "DoorComponent requires collider";

	entity->collider->SetKinematic(true);
	entity->collider->SetAutosleep(false);

	closedPosition = entity->collider->GetPosition();
	openPosition = closedPosition + entity->GetGlobalUp()*entity->collider->dimensions.y;
	isOpen = false;
}

void DoorComponent::OnMessage(const std::string& msg, const OpaqueType& ot){
	if(msg == "forceopen") {
		switchStates = ~0u;
		isOpen = true;
		mover->MoveTo(openPosition, 0.001f);

	}else if(msg == "open"){
		switchStates = ~0u;
		UpdateState();
		// If a component was passed as an argument, notify it that
		//	the door is opening
		if(auto trigcomp = ot.Get<Component*>(false)){
			(*trigcomp)->entity->SendMessage("dooropen", (Component*)this);
		}

	}else if(msg == "close"){
		switchStates = 0;
		UpdateState();
		// If a component was passed as an argument, notify it that
		//	the door is closing
		if(auto trigcomp = ot.Get<Component*>(false)){
			(*trigcomp)->entity->SendMessage("doorclose", (Component*)this);
		}

	}else if(msg == "unlock"){
		std::tuple<Component*, u32>* args = ot.Get<Component*, u32>(true);
		auto comp = std::get<0>(*args); // The component that sent the message
		auto num = std::get<1>(*args); // The number of the key (bell, switch)

		// Flag the key as used
		switchStates |= (1<<num) & requiredMask;

		if(ordered){
			bool po2 = false;

			// This ensures that keys aren't used in the
			//	wrong order
			for(u32 i = 0; i < 30; ++i){
				// Since bits should always be set from the right
				//	switchStates+1 if in the correct order, should
				//	always be a single bit
				po2 = ((switchStates+1) == 1u<<i);
				if(po2) break;
			}

			// switchStates+1 isn't a power of 2, the combination was 
			//	obviously wrong
			if(!po2) {
				switchStates = 0;
				std::cout << "Invalid combination" << std::endl;
			}
		}

		UpdateState();

		// If a sender component was specified (should always be true)
		if(comp){
			// and the door hasn't been opened yet
			if(!isOpen) {
				// Inform the sender component of whether or not
				//	it was correct
				comp->entity->SendMessage((switchStates==0)?"incorrect":"correct", (Component*)this);
			}else{
				// Otherwise, the door has been opened, so notify the sender
				comp->entity->SendMessage("dooropen", (Component*)this);
			}
		}
	}
}

void DoorComponent::UpdateState() {
	auto player = App::GetSingleton()->player;

	// If all required switch states are set
	if ((switchStates&requiredMask) == requiredMask) {
		// Do the things
		if (!isOpen) {
			mover->MoveTo(openPosition, openTime);
		}

		isOpen = true;
		player->shakeCount--; // Manpat: I don't like this

	} else {
		if(isOpen) {
			mover->MoveTo(closedPosition, openTime);
		}

		isOpen = false;
		player->shakeCount--; // Manpat: I don't like this
	}
}