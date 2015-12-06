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

	FMOD::DSP* postcompressor = nullptr;
	FMOD::DSP* precompressor = nullptr;
	cfmod(system->createDSPByType(FMOD_DSP_TYPE_COMPRESSOR, &postcompressor));
	cfmod(postcompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, -6));
	cfmod(postcompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK, 20.0));
	cfmod(postcompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_RELEASE, 50.0));
	cfmod(postcompressor->setBypass(false));

	cfmod(system->createDSPByType(FMOD_DSP_TYPE_COMPRESSOR, &precompressor));
	cfmod(precompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, -6));
	cfmod(precompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK, 100.0));
	cfmod(precompressor->setParameterFloat(FMOD_DSP_COMPRESSOR_RELEASE, 100.0));
	cfmod(precompressor->setBypass(false));

	cfmod(system->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &reverb));

	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, 10.));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, 10.));

	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, 50.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, 100.0));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, 1000.0));
	cfmod(reverb->setBypass(false));
	SetReverbMix(0.f);

	cfmod(system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass));
	cfmod(lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000.0));
	cfmod(lowPass->setBypass(false));

	// precompressor -> reverb -> lowPass -> postcompressor
	cfmod(mastergroup->addDSP(0, postcompressor));
	cfmod(mastergroup->addDSP(0, lowPass));
	cfmod(mastergroup->addDSP(0, reverb));
	cfmod(mastergroup->addDSP(0, precompressor));
}

AudioManager::~AudioManager() {
	system->release();
}

void AudioManager::Update() {
	const f32 smoothing = 40.f;
	lowPassAmt = (lowPassAmt * (smoothing-1.f) + targetLowPassAmt) / smoothing;
	reverbTime = (reverbTime * (smoothing-1.f) + targetReverbTime) / smoothing;
	reverbMix = (reverbMix * (smoothing-1.f) + targetReverbMix) / smoothing;

	cfmod(lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lowPassAmt));
	cfmod(reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, reverbTime));
	cfmod(reverb->setWetDryMix(1.0, 1.0, reverbMix));

	cfmod(system->update());
}

void AudioManager::SetLowpass(f32 v){
	targetLowPassAmt = v;
}

void AudioManager::SetReverbMix(f32 mx){
	targetReverbMix = mx;
}

void AudioManager::SetReverbTime(f32 ms){
	targetReverbTime = ms;
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