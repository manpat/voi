#include "physicsmanager.h"
#include "movableobject.h"
#include "entity.h"

void Movable::NotifyPickup() {
	entity->collider->SetKinematic(true);
	entity->collider->SetAutosleep(false);
	entity->collider->Wakeup();
}

void Movable::NotifyDrop() {
	entity->collider->SetKinematic(false);
	entity->collider->SetAutosleep(true);
	entity->collider->Wakeup();
}