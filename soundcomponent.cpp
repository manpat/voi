#include "entity.h"
#include "audiomanager.h"
#include "soundcomponent.h"

SoundComponent::SoundComponent(const std::string& fn) : Component(this), fileName(fn) {}

void SoundComponent::OnInit() {
	auto audioMan = AudioManager::GetSingleton();

	cfmod(audioMan->system->createSound(fileName.data(), FMOD_LOOP_NORMAL|FMOD_3D, nullptr, &sound));
	cfmod(audioMan->system->playSound(sound, audioMan->mastergroup, false, &channel));

	cfmod(channel->setMode(FMOD_3D));
	// TODO: Figure out values properly
	// e.g. test if entity has collider and use dimensions
	cfmod(channel->set3DMinMaxDistance(1.0f, 10000.0f));
}

void SoundComponent::OnUpdate() {
	auto pos = o2fm(entity->GetGlobalPosition());
	auto vel = o2fm(vec3::ZERO);

	// Set position
	cfmod(channel->set3DAttributes(&pos, &vel, nullptr));
}

void SoundComponent::OnDestroy() {
	sound->release();
}