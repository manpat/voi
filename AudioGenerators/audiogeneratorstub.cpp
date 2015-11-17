#include "testaudiogenerators.h"
#include "ambience.h"
#include "hub.h"

#include "../audiomanager.h"
#include "../bellmanager.h"

void InitAudioGenerators(std::shared_ptr<AudioManager> audioManager) {
	audioManager->RegisterAudioGeneratorType<DoorAudioGenerator>("door");
	audioManager->RegisterAudioGeneratorType<TrophyAudioGenerator>("trophy");
	audioManager->RegisterAudioGeneratorType<FourWayAudioGenerator>("4way");
	audioManager->RegisterAudioGeneratorType<HighArpeggiatorAudioGenerator>("higharp");
	audioManager->RegisterAudioGeneratorType<LowArpeggiatorAudioGenerator>("lowarp");

	audioManager->RegisterAudioGeneratorType<NoiseAudioGenerator>("noise");
	audioManager->RegisterAudioGeneratorType<LowRumbleAudioGenerator>("lowrumble");
	audioManager->RegisterAudioGeneratorType<ChoirAudioGenerator>("choir");

	audioManager->RegisterAudioGeneratorType<HubAudioGenerator>("hub");
	BellManager::RegisterAudio();
}