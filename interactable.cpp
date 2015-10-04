#include "entitymanager.h"
#include "interactable.h"
#include "entity.h"
#include <iostream>

void Interactable::Activate(){
	std::cout << "Interactable::Activate " << entity->GetName() << std::endl;

	if(target){
		target->SendMessage(action, (Component*)this);
	}else{
		entity->SendMessage(action, (Component*)this);
	}
}

void Interactable::OnAwake(){
	if(!target && targetName.size() > 0){
		target = EntityManager::GetSingleton()->FindEntity(targetName);
		if(target)
			std::cout << "Interactable found target " << target->GetName() << std::endl;
		else
			std::cout << "Interactable target name not valid" << std::endl;
	}
}