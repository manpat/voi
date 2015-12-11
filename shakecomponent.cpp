#include "shakecomponent.h"
#include "apptime.h"
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
	if (timerSeconds > AppTime::appTime) {
		player->ShakeCamera(fadeShakeAmount * (f32)fmin(timerSeconds - AppTime::appTime, fadeSeconds));
	}

	// This should override timer
	if (isShaking) {
		player->ShakeCamera(0.02f);
	}
}

// Shake amount, time to shake in seconds exclusive of decay, optional time to decay in seconds
void ShakeComponent::SetTimer(f32 shakeAmount, f64 time, f64 fadeTime) {
	if (AppTime::appTime + time > timerSeconds) {
		timerSeconds = AppTime::appTime + time + fadeTime;
		fadeSeconds = fadeTime;
		fadeShakeAmount = shakeAmount;
	}
}
