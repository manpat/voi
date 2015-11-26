#include "synthcomponent.h"
#include "entitymanager.h"
#include "interactable.h"
#include "bellmanager.h"
#include "entity.h"
#include "bell.h"

#include "AudioGenerators/bell.h"

void Bell::OnAwake() {
	entity->AddComponent<Interactable>();

	target = EntityManager::GetSingleton()->FindEntity(targetName);
	if(!target){
		std::cout << "Bell " << entity->GetName()
			<< " didn't find target " << targetName << std::endl;
	}

	auto synth = entity->FindComponent<SynthComponent>();
	if(synth && synth->synthName == "bell") {
		static u32 notes[] = {
			0,
			4,
			7,
			11,

			12,
			16,
			19,
			23,
		};

		bellGen = std::dynamic_pointer_cast<BellAudioGenerator>(synth->generator);
		bellGen->note = 120 + notes[bellNumber];
	}else{
		std::cout << "WARNING!! Bell missing audio generator" << std::endl;
	}

	BellManager::GetSingleton()->AddBell(targetName, this);
}

void Bell::OnMessage(const std::string& msg, const OpaqueType&) {
	if(!target) return;

	if(msg == "interact"){
		std::cout << "Bell unlock " << bellNumber << std::endl;
		if(bellGen){
			bellGen->Trigger();
		}
		target->SendMessage("unlock", (Component*)this, /*u32*/bellNumber);

	}else if(msg == "correct") {
		std::cout << "Bell unlock " << bellNumber << " correct" << std::endl;

	}else if(msg == "incorrect") {
		std::cout << "Bell unlock " << bellNumber << " incorrect" << std::endl;
		BellManager::GetSingleton()->StopAllBells(targetName);

	}else if(msg == "dooropen") {
		std::cout << "Bell unlock " << bellNumber << " dooropen" << std::endl;
		BellManager::GetSingleton()->CorrectCombination(targetName);
	}
}