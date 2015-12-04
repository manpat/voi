#include <OGRE/OgreSceneNode.h>
#include <limits>

#include "halflifepointcomponent.h"
#include "halflifepointmanager.h"
#include "physicsmanager.h"
#include "entitymanager.h"
#include "audiomanager.h"
#include "hubmanager.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

template<> HalfLifePointManager* Singleton<HalfLifePointManager>::instance = nullptr;

HalfLifePointManager::HalfLifePointManager() {

}

void HalfLifePointManager::TriggerSceneLoad(HalfLifePointComponent* atc, vec3 o, quat q) {
	posOffset = o;
	rotOffset = q;
	toLevel = atc->toLevel;
	toNode = atc->entity->GetName();
}

void HalfLifePointManager::Update() {
	// Walk out of half-life point
	if (toLevel.size() > 0) {
		auto app = App::GetSingleton();
		auto entMgr = EntityManager::GetSingleton();

		std::cout << "LOAD LEVEL " << toLevel << "!!!!!!" << std::endl;
		std::cout << "Goto node " << toNode << " with offset " << posOffset << std::endl;

		app->Load(toLevel);

		vec3 spawnPos = vec3::ZERO;
		quat spawnRot = quat::IDENTITY;

		auto spawnNode = entMgr->FindEntity(toNode);
		if (!spawnNode) {
			std::cout << "Couldn't find entity in new level " + toNode;
		} else {
			auto spawnNodeOri = vec3::UNIT_Z.getRotationTo(spawnNode->GetWorldPlaneFromMesh().normal);

			spawnPos = spawnNodeOri * posOffset + spawnNode->GetGlobalPosition();
			spawnRot = rotOffset * spawnNodeOri;
		}

		app->player->entity->collider->SetPosition(spawnPos);
		app->player->SetToOrientation(spawnRot);
		
		if(toLevel == "hub")
			HubManager::GetSingleton()->NotifyReturnToHub();

		toLevel = "";
	}

	auto audioMan = AudioManager::GetSingleton();

	const f32 maxDist = 20.0;
	if(minDistance <= maxDist){
		auto a = std::max((minDistance-maxDist/3.f)/maxDist*2.f/3.f, 0.f); // (0, 1)
		auto b = std::max(1.0f-minDistance/maxDist, 0.0f); // (0, 1)

		audioMan->SetLowpass(a*a*a*22000.0f+20.0f);
		audioMan->SetReverbMix(b);
		audioMan->SetReverbTime(b*20000.0f);
	}else{
		audioMan->SetLowpass(22000.0f);
		audioMan->SetReverbTime(1000.0f);
		audioMan->SetReverbMix(5.f);
	}

	minDistance = std::numeric_limits<f32>::max();
}

void HalfLifePointManager::ProcessPointDistance(f32 distance) {
	minDistance = std::min(distance, minDistance);
}
