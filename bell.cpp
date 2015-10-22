#include "audiogenerator.h"
#include "entitymanager.h"
#include "audiomanager.h"
#include "entity.h"
#include "bell.h"

void Bell::OnAwake() {
	target = EntityManager::GetSingleton()->FindEntity(targetName);
	if(!target){
		std::cout << "Bell " << entity->GetName() 
			<< " didn't find target " << targetName << std::endl;
	}
}

// struct BellAudioGenerator : AudioGenerator {
// 	f32 Generate(f64 elapsed) override {
// 		auto A = ntof(128);
// 		auto o = Wave::Sine(elapsed * A);

// 		return o;
// 	}
// };

void Bell::RegisterAudio() {
	// AudioManager::GetSingleton()->RegisterAudioGeneratorType<BellAudioGenerator>("bell");	
}
