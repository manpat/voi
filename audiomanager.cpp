#include "audiomanager.h"

void cfmod(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
		throw "FMOD Error";
	}
}

AudioManager::AudioManager() {
	cfmod(FMOD::System_Create(&system));

	synth = new AudioSynth();

	u32 version = 0;
	cfmod(system->getVersion(&version));
	if (version < FMOD_VERSION) {
		std::cerr
			<< "FMOD version of at least " << FMOD_VERSION
			<< " required. Version used " << version
			<< std::endl;
		throw "FMOD Error";
	}

	cfmod(system->init(100, FMOD_INIT_NORMAL, nullptr));
	cfmod(system->getMasterChannelGroup(&mastergroup));

	running = true;
}

AudioManager::~AudioManager() {
	running = false;
}

void AudioManager::update(u64 dt) {
	cfmod(system->update());
}

void AudioManager::playNote(u8 degree) {
	synth->scheduler->NoteOn(degree);
}

FmodSystemRef AudioManager::getSystem() {
	return FmodSystemRef{ system };
}