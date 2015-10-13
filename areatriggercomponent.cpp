#include "areatriggercomponent.h"
#include "areatriggermanager.h"
#include "physicsmanager.h"
#include "entity.h"
#include "player.h"

void AreaTriggerComponent::OnTriggerEnter(ColliderComponent* o) {
	if(auto p = o->entity->FindComponent<Player>()){
		auto ppos = p->entity->collider->GetPosition();
		auto tpos = entity->collider->GetPosition();
		AreaTriggerManager::GetSingleton()->TriggerSceneLoad(this, ppos - tpos);
	}
}
