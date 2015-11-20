#include "physicsmanager.h"
#include "checkpoint.h"
#include "entity.h"
#include "player.h"
#include "app.h"

#define PRINT(msg) std::cout << "Checkpoint: " << msg << std::endl;

void Checkpoint::OnInit() {
	PRINT("OnInit()");
}

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