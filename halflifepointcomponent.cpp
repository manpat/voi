#include "halflifepointcomponent.h"
#include "halflifepointmanager.h"
#include "physicsmanager.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

void HalfLifePointComponent::OnUpdate(){
	auto player = App::GetSingleton()->player;
	auto ppos = player->entity->collider->GetPosition();
	auto pos = entity->collider->GetPosition();
	auto dist = (f32)(ppos-pos).length();

	HalfLifePointManager::GetSingleton()->ProcessPointDistance(dist);
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