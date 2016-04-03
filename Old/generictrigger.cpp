#include "generictrigger.h"
#include "entitymanager.h"
#include "entity.h"
#include <iostream>

void GenericTrigger::OnAwake(){
	if(!target && targetName.size() > 0){
		target = EntityManager::GetSingleton()->FindEntity(targetName);
		if(!target){
			std::cout << "GenericTrigger " << entity->GetName()
				<< " didn't find target " << targetName << std::endl;
		}
	}
}

void GenericTrigger::OnTriggerEnter(ColliderComponent*){
	if(target){
		target->SendMessage(action, (Component*)this);
	}

	// Send message to self regardless
	entity->SendMessage(action, (Component*)this);
}