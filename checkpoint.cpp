#include "physicsmanager.h"
#include "checkpoint.h"
#include "entity.h"
#include "player.h"
#include "camera.h"
#include "app.h"

#define PRINT(msg) std::cout << "Checkpoint: " << msg << std::endl;

void Checkpoint::OnTriggerEnter(ColliderComponent* co) {
	auto player = co->entity->FindComponent<Player>();

	if (player != nullptr) {
		auto app = App::GetSingleton();

		// Fetch global player position and camera orientation
		playerSpawnPosition = player->entity->GetGlobalPosition();
		playerSpawnOrientation = vec3::UNIT_Z.getRotationTo(entity->GetWorldPlaneFromMesh().normal); // Get orientation from normal of checkpoint plane

		app->currentCheckpoint = this;
	}
}

void Checkpoint::OnDestroy() {
	auto& cc = App::GetSingleton()->currentCheckpoint;

	if (cc == this) {
		cc = nullptr;
	}
}