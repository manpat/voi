#include "synthcomponent.h"
#include "audiogenerator.h"
#include "entitymanager.h"
#include "audiomanager.h"
#include "interactable.h"
#include "entity.h"
#include "bell.h"

struct BellAudioGenerator : AudioGenerator {
	u32 note = 120;
	f64 savedElapsed = -1.0;
	bool triggered = false;

	f32 Generate(f64 elapsed) override {
		if(triggered){
			savedElapsed = elapsed;
			triggered = false;
		}

		f32 env = 1.0 - clamp((elapsed-savedElapsed)/4.0, 0.0, 1.0);
		auto f = ntof(note);
		auto o = Wave::Sin(elapsed * f) * (env + 0.1);

		return o;
	}
};

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
			5,
			8,
			12,
		};

		bellGen = std::dynamic_pointer_cast<BellAudioGenerator>(synth->generator);
		bellGen->note = 120 + notes[bellNumber];
	}else{
		std::cout << "WARNING!! Bell missing audio generator" << std::endl;
	}
}

void Bell::OnMessage(const std::string& msg, const OpaqueType&) {
	if(target && msg == "interact"){
		target->SendMessage("unlock"+std::to_string(bellNumber), (Component*)this);
		std::cout << "Bell unlock " << bellNumber << std::endl;
		if(bellGen){
			bellGen->triggered = true;
		}
	}
}

void Bell::RegisterAudio() {
	AudioManager::GetSingleton()->RegisterAudioGeneratorType<BellAudioGenerator>("bell");
}
