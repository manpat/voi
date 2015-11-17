#include "entity.h"
#include "audiomanager.h"
#include "soundcomponent.h"

SoundComponent::SoundComponent(const std::string& fn, f32 s) : Component(this), fileName(fn), size(s) {}

void SoundComponent::OnInit() {
	auto audioMan = AudioManager::GetSingleton();

	cfmod(audioMan->system->createSound(fileName.data(), FMOD_LOOP_NORMAL|FMOD_3D, nullptr, &sound));
	cfmod(audioMan->system->playSound(sound, audioMan->mastergroup, true, &channel));

	cfmod(channel->setMode(FMOD_3D));
	// TODO: Figure out values properly
	// e.g. test if entity has collider and use dimensions
	cfmod(channel->set3DMinMaxDistance(size, 10000.0f));
}

void SoundComponent::OnAwake(){
	// Unpausing in OnAwake rather than in OnInit means that the entity
	//	has a chance to move to where it should be
	cfmod(channel->setPaused(false));
}

void SoundComponent::OnUpdate() {
	auto pos = o2fm(entity->GetGlobalPosition());
	auto vel = o2fm(vec3::ZERO);

	// Set position
	cfmod(channel->set3DAttributes(&pos, &vel, nullptr));
}

void SoundComponent::OnDestroy() {
	channel->stop();
	sound->release();
}