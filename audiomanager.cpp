#include "audiomanager.h"
#include "entity.h"
#include "camera.h"
#include "app.h"

void cfmod(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		std::cerr << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
		throw "FMOD Error";
	}
}

FMOD_VECTOR o2fm(const vec3& o){
	return FMOD_VECTOR{o.x, o.y, o.z};
}

template<> AudioManager* Singleton<AudioManager>::instance = nullptr;

AudioManager::AudioManager() {
	cfmod(FMOD::System_Create(&system));

	u32 version = 0;
	cfmod(system->getVersion(&version));
	if (version < FMOD_VERSION) {
		std::cerr
			<< "FMOD version of at least " << FMOD_VERSION
			<< " required. Version used " << version
			<< std::endl;
		throw "FMOD Error";
	}

	cfmod(system->init(100, FMOD_INIT_3D_RIGHTHANDED|FMOD_INIT_CHANNEL_LOWPASS, nullptr));
	cfmod(system->getMasterChannelGroup(&mastergroup));

	FMOD::DSP* compressor = nullptr;
	cfmod(system->createDSPByType(FMOD_DSP_TYPE_COMPRESSOR, &compressor));
	cfmod(compressor->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, -13));
	cfmod(compressor->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK, 0.5));
	cfmod(compressor->setBypass(false));
	cfmod(mastergroup->addDSP(0, compressor));

	cfmod(system->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &reverb));

	// cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, -60.0));

	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, 10.));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, 10.));

	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, 1000.0));
	cfmod(reverb->setBypass(false));
	cfmod(mastergroup->addDSP(1, reverb));
	SetReverbMix(0.f);

	cfmod(system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass));
	cfmod(lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000.0));
	cfmod(lowPass->setBypass(false));
	cfmod(mastergroup->addDSP(2, lowPass));
}

AudioManager::~AudioManager() {
	system->release();
}

void AudioManager::Update() {
	cfmod(system->update());
}

void AudioManager::SetLowpass(f32 v){
	cfmod(lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, v));
}

void AudioManager::SetReverbMix(f32 mx){
	reverb->setWetDryMix(1.0, 1.0, mx);
}

void AudioManager::SetReverbTime(f32 ms){
	reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, ms);
}

std::shared_ptr<AudioGenerator> AudioManager::CreateAudioGenerator(const std::string& name){
	auto agf = findin(audioGeneratorTemplates, name);
	if(!agf) throw "Tried to create AudioGenerator of non-existent type: " + name;

	return agf->Create();
}


AudioListenerComponent::AudioListenerComponent() : Component(this) {}

// TODO: Fuck this off when camera gets better
#include <OGRE/OgreSceneNode.h>

void AudioListenerComponent::OnUpdate() {
	auto cam = App::GetSingleton()->camera;
	auto sys = AudioManager::GetSingleton()->system;
	
	// TODO: Fuck this off when camera gets better
	auto ori = cam->cameraNode->_getDerivedOrientation();

	auto pos = o2fm(entity->GetGlobalPosition());
	auto vel = o2fm(vec3::ZERO);
	auto fwd = o2fm(-ori.zAxis());
	auto up = o2fm(ori.yAxis());

	cfmod(sys->set3DListenerAttributes(0, &pos, &vel, &fwd, &up));
}