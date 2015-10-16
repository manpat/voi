#include <OGRE\OgreSceneNode.h>

#include "areatriggercomponent.h"
#include "areatriggermanager.h"
#include "physicsmanager.h"
#include "entitymanager.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

template<> AreaTriggerManager* Singleton<AreaTriggerManager>::instance = nullptr;

AreaTriggerManager::AreaTriggerManager() {

}

void AreaTriggerManager::TriggerSceneLoad(AreaTriggerComponent* atc, vec3 o) {
	posOffset = o;
	toLevel = atc->toLevel;
	toNode = atc->entity->GetName();
}

void AreaTriggerManager::Update() {
	if(toLevel.size() > 0){
		auto app = App::GetSingleton();
		auto entMgr = EntityManager::GetSingleton();

		std::cout << "LOAD LEVEL " << toLevel << "!!!!!!" << std::endl;
		std::cout << "Goto node " << toNode << " with offset " << posOffset << std::endl;

		app->Load(toLevel);
		vec3 spawnPos = vec3::ZERO;


		auto spawnNode = entMgr->FindEntity(toNode);
		if(!spawnNode) std::cout << "Couldn't find entity in new level " + toNode;
		else {
			spawnPos = spawnNode->GetGlobalPosition() + posOffset;
		}

		app->player->entity->collider->SetPosition(spawnPos);
		// TODO: Orientation
		//app->camera->cameraNode->set

		toLevel = "";
	}
}
