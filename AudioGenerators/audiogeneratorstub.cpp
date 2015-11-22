#include "testaudiogenerators.h"
#include "ambience.h"
#include "hub.h"
#include "sfx.h"

#include "../audiomanager.h"
#include "../bellmanager.h"

void InitAudioGenerators(std::shared_ptr<AudioManager> audioManager) {
	// TODO: Remove these
	audioManager->RegisterAudioGeneratorType<HighArpeggiatorAudioGenerator>("higharp");
	audioManager->RegisterAudioGeneratorType<LowArpeggiatorAudioGenerator>("lowarp");
	audioManager->RegisterAudioGeneratorType<TrophyAudioGenerator>("trophy");
	audioManager->RegisterAudioGeneratorType<FourWayAudioGenerator>("4way");

	audioManager->RegisterAudioGeneratorType<LowRumbleAudioGenerator>("lowrumble");
	audioManager->RegisterAudioGeneratorType<DoorGrindAudioGenerator>("door");
	audioManager->RegisterAudioGeneratorType<NoiseAudioGenerator>("noise");
	audioManager->RegisterAudioGeneratorType<ChoirAudioGenerator>("choir");

	audioManager->RegisterAudioGeneratorType<HubAudioGenerator>("hub");
	BellManager::RegisterAudio();
}