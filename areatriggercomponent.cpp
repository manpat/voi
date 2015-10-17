#include "areatriggercomponent.h"
#include "areatriggermanager.h"
#include "physicsmanager.h"
#include "audiomanager.h"
#include "entity.h"
#include "player.h"
#include "app.h"

void AreaTriggerComponent::OnUpdate(){
	auto audioMan = AudioManager::GetSingleton();
	auto player = App::GetSingleton()->player;
	auto ppos = player->entity->collider->GetPosition();
	auto pos = entity->collider->GetPosition();
	auto dist = (ppos-pos).length();

	if(dist < 30.0){
		auto a = std::max((dist-10.0)/30.0, 0.0); // (0, 1)
		audioMan->SetLowpass(a*a*22000.0+20.0);
	}else{
		// Assumes that there's only one area trigger component
		audioMan->SetLowpass(22000.0);
	}
}

void AreaTriggerComponent::OnTriggerEnter(ColliderComponent* o) {
	if(auto p = o->entity->FindComponent<Player>()){
		auto ppos = p->entity->collider->GetPosition();
		auto tpos = entity->collider->GetPosition();
		AreaTriggerManager::GetSingleton()->TriggerSceneLoad(this, ppos - tpos);
	}
}
