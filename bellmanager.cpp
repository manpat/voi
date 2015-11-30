#include "AudioGenerators/bell.h"
#include "audiomanager.h"
#include "bellmanager.h"
#include "bell.h"

template<>
BellManager* Singleton<BellManager>::instance = nullptr;

void BellManager::AddBell(const std::string& target, Bell* b) {
	if(!existsin(bells, target)) {
		bells[target] = {b};
		return;
	}

	bells[target].push_back(b);
}
void BellManager::StopAllBells(const std::string& target) {
	for(auto& b: bells[target]) {
		if(b->bellGen)
			b->bellGen->Stop();
	}
}

void BellManager::CorrectCombination(const std::string& target) {
	std::cout << "CorrectCombination" << std::endl;
	for(auto& b: bells[target]) {
		if(b->bellGen){
			// b->bellGen->note += 12;
			b->bellGen->SetParam(1, 1);
			// b->bellGen->Start();
			// b->bellGen->Stop();
		}
	}
}