#include "shakecomponent.h"
#include "entity.h"
#include "player.h"
#include "app.h"

#include <cassert>

#define PRINT(msg) std::cout << "ShakeCom: " << msg << std::endl;

void ShakeComponent::OnInit() {
	player = App::GetSingleton()->player;
	assert(player != nullptr);
}

void ShakeComponent::OnUpdate() {
	if (isShaking) {
		player->ShakeCamera(0.02f);
	}
}