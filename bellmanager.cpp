#include "AudioGenerators/bell.h"
#include "audiomanager.h"
#include "bellmanager.h"
#include "bell.h"

template<>
BellManager* Singleton<BellManager>::instance = nullptr;

void BellManager::AddBell(Bell* b) {
	bells.push_back(b);
}
void BellManager::StopAllBells() {
	for(auto& b: bells) {
		if(b->bellGen)
			b->bellGen->Stop();
	}
}

void BellManager::CorrectCombination() {
	std::cout << "CorrectCombination" << std::endl;
	for(auto& b: bells) {
		if(b->bellGen){
			b->bellGen->note += 12;
			b->bellGen->Trigger();
			b->bellGen->Stop();
		}
	}
}