#include "interactable.h"
#include "entity.h"
#include <iostream>

void Interactable::Activate(){
	std::cout << "Interactable::Activate " << entity->GetName() << std::endl;
}