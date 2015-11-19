#include "entitymanager.h"
#include "hubmanager.h"
#include "entity.h"

template<>
HubManager* Singleton<HubManager>::instance = nullptr;

void HubManager::NotifyReturnToHub() {
	doneMyThang = false;
	if(lastLevelCompleted < 2)
		lastLevelCompleted++;
	else
		lastLevelCompleted = 3;
}

void HubManager::Update() {
	if(doneMyThang || lastLevelCompleted < 0) return;
	doneMyThang = true;

	auto entmgr = EntityManager::GetSingleton();

	Entity* stairs[] = {
		entmgr->FindEntity("Stairs.Level1"),
		entmgr->FindEntity("Stairs.Level2"),
		entmgr->FindEntity("Stairs.Level3"),
		entmgr->FindEntity("Stairs.Level4"),
	};

	for(s32 i = 0; i < lastLevelCompleted; i++) {
		stairs[i]->SetLayer(0);
		stairs[i]->SendMessage("forceopen");
	}

	stairs[lastLevelCompleted]->SetLayer(0);
	stairs[lastLevelCompleted]->SendMessage("open", (Component*)nullptr);

	if(lastLevelCompleted == 3) {
		auto door = entmgr->FindEntity("FinalDoor");
		door->SendMessage("open", (Component*)nullptr);
	}
}