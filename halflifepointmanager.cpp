#include <OGRE/OgreSceneNode.h>

#include "halflifepointcomponent.h"
#include "halflifepointmanager.h"
#include "physicsmanager.h"
#include "entitymanager.h"
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
			spawnRot = spawnNodeOri + rotOffset;
		}

		app->player->entity->collider->SetPosition(spawnPos);
		app->player->SetToOrientation(spawnRot);

		toLevel = "";
	}
}
