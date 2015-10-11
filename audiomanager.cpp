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
}

AudioManager::~AudioManager() {
	system->release();
}

void AudioManager::Update() {
	cfmod(system->update());
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