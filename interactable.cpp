#include "entitymanager.h"
#include "interactable.h"
#include "entity.h"
#include <iostream>

void Interactable::Activate(){
	if(target){
		target->SendMessage(action, (Component*)this);
	}

	// Send message to self regardless
	entity->SendMessage(action, (Component*)this);
}

void Interactable::OnAwake(){
	if(!target && targetName.size() > 0){
		target = EntityManager::GetSingleton()->FindEntity(targetName);
		if(!target){
			std::cout << "Interactable " << entity->GetName()
				<< " didn't find target " << targetName << std::endl;
		}
	}
}