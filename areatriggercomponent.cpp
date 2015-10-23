#include "areatriggercomponent.h"
#include "areatriggermanager.h"
#include "physicsmanager.h"
#include "audiomanager.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

void AreaTriggerComponent::OnUpdate(){
	auto audioMan = AudioManager::GetSingleton();
	auto player = App::GetSingleton()->player;
	auto ppos = player->entity->collider->GetPosition();
	auto pos = entity->collider->GetPosition();
	auto dist = (ppos-pos).length();

	if(dist <= 40.0){
		auto a = std::max((dist-10.0f)/30.0f, 0.0f); // (0, 1)
		auto b = std::max(1.0f-dist/40.0f, 0.0f); // (0, 1)

		audioMan->SetLowpass(a*a*a*22000.0f+20.0f);
		audioMan->SetReverbMix(b);
		audioMan->SetReverbTime(b*20000.0f);
	}else{
		// Assumes that there's only one area trigger component
		audioMan->SetLowpass(22000.0);
		audioMan->SetReverbMix(0.0);
	}
}

void AreaTriggerComponent::OnTriggerEnter(ColliderComponent* o) {
	if(auto p = o->entity->FindComponent<Player>()){
		auto ppos = p->entity->collider->GetPosition();
		auto tpos = entity->collider->GetPosition();

		auto camera = App::GetSingleton()->camera;
		auto prot = camera->entity->GetGlobalOrientation();
		auto trot = entity->GetGlobalOrientation();

		auto rotdiff = prot * trot.Inverse();

		AreaTriggerManager::GetSingleton()->TriggerSceneLoad(this, ppos - tpos, rotdiff);
	}
}
