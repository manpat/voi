#include "halflifepointcomponent.h"
#include "halflifepointmanager.h"
#include "physicsmanager.h"
#include "audiomanager.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

void HalfLifePointComponent::OnUpdate(){
	auto audioMan = AudioManager::GetSingleton();
	auto player = App::GetSingleton()->player;
	auto ppos = player->entity->collider->GetPosition();
	auto pos = entity->collider->GetPosition();
	auto dist = (f32)(ppos-pos).length();

	const f32 maxDist = 20.0;
	if(dist <= maxDist){
		auto a = std::max((dist-maxDist/3.f)/maxDist*2.f/3.f, 0.f); // (0, 1)
		auto b = std::max(1.0f-dist/maxDist, 0.0f); // (0, 1)

		auto lp = audioMan->targetLowPassAmt;
		auto rm = audioMan->targetReverbMix;
		auto rt = audioMan->targetReverbTime;

		audioMan->SetLowpass(std::min(lp, a*a*a*22000.0f+20.0f));
		audioMan->SetReverbMix(std::max(rm, b));
		audioMan->SetReverbTime(std::max(rt, b*20000.0f));
	}else{
		// Assumes that there's only one area trigger component
		audioMan->SetLowpass(22000.0);
		audioMan->SetReverbMix(0.0);
	}
}

void HalfLifePointComponent::OnTriggerEnter(ColliderComponent* o) {
	// Walk into half-life point
	if (auto p = o->entity->FindComponent<Player>()) {
		auto ppos = p->entity->collider->GetPosition();
		auto tpos = entity->collider->GetPosition();

		auto camera = App::GetSingleton()->camera;
		auto prot = camera->entity->GetGlobalOrientation();
		auto trot = vec3::UNIT_Z.getRotationTo(entity->GetWorldPlaneFromMesh().normal);

		HalfLifePointManager::GetSingleton()->TriggerSceneLoad(this, ppos - tpos, prot * trot.Inverse());
	}
}