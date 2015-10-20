#include "physicsmanager.h"
#include "checkpoint.h"
#include "entity.h"
#include "player.h"
#include "app.h"

void Checkpoint::OnTriggerEnter(ColliderComponent* co) {
	if(co->entity->FindComponent<Player>()) {
		App::GetSingleton()->currentCheckpoint = this;
	}
}

void Checkpoint::OnDestroy() {
	auto& cc = App::GetSingleton()->currentCheckpoint;
	if(cc == this){
		cc = nullptr;
	}
}