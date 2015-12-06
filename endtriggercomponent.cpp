#include "endtriggercomponent.h"
#include "app.h"

void EndTriggerComponent::OnTriggerEnter(ColliderComponent*) {
	if(hasTriggered) return;

	App::GetSingleton()->NotifyEndGame();
	hasTriggered = true;
}

