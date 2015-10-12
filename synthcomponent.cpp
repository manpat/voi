#include "entity.h"
#include "audiomanager.h"
#include "audiogenerator.h"
#include "synthcomponent.h"

SynthComponent::SynthComponent(const std::string& sn, f32 s) : Component(this), synthName(sn), size(s) {}

void SynthComponent::OnInit() {
	auto audioMan = AudioManager::GetSingleton();

	generator = audioMan->CreateAudioGenerator(synthName);

	FMOD_DSP_DESCRIPTION desc;
	memset(&desc, 0, sizeof(desc));

	desc.numinputbuffers = 0;
	desc.numoutputbuffers = 1;
	desc.read = GeneratorFunction;
	desc.userdata = this;

	cfmod(audioMan->system->createDSP(&desc, &dsp));
	cfmod(dsp->setChannelFormat(FMOD_CHANNELMASK_MONO, 1, FMOD_SPEAKERMODE_MONO));

	cfmod(audioMan->system->playDSP(dsp, audioMan->mastergroup, true /*start paused*/, &channel));
	cfmod(channel->setMode(FMOD_3D));
	cfmod(channel->set3DMinMaxDistance(size, 10000.0f));

	auto newReverb = [&]() -> FMOD::DSP* {
		FMOD::DSP* rvb;
		cfmod(audioMan->system->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &rvb));
		cfmod(channel->addDSP(1, rvb));

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, -60.0);

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, 100.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, 10000.);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, 10.);

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, 100.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, 100.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, 40.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, 1000.0);

		return rvb;
	};

	reverb = newReverb();
}

void SynthComponent::OnAwake(){
	// Unpausing in OnAwake rather than in OnInit means that the entity 
	//	has a chance to move to where it should be
	cfmod(channel->setPaused(false));
}

void SynthComponent::OnUpdate() {
	auto pos = o2fm(entity->GetGlobalPosition());
	auto vel = o2fm(vec3::ZERO);

	// Set position
	cfmod(channel->set3DAttributes(&pos, &vel, nullptr));
}

void SynthComponent::OnDestroy() {
	dsp->release();
}

f32 SynthComponent::Generate(f64 dt) {
	auto o = generator->Generate(elapsed);

	elapsed += dt;
	return o;
}

void SynthComponent::SetReverbTime(f32 ms){
	reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, ms);
}

void SynthComponent::SetReverbMix(f32 mx){
	reverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, mx);
}

FMOD_RESULT F_CALLBACK 
SynthComponent::GeneratorFunction(FMOD_DSP_STATE* state, f32*, f32* outbuffer, u32 length, s32, s32*){
	
	s32 samplerate = 0;
	cfmod(state->callbacks->getsamplerate(state, &samplerate));
	f64 inc = 1.0 / samplerate;

	FMOD::DSP *thisdsp = (FMOD::DSP *)state->instance;

	void* ud = nullptr;
	cfmod(thisdsp->getUserData(&ud));
	auto synth = static_cast<SynthComponent*>(ud);

	for (u32 i = 0; i < length; ++i) {
		outbuffer[i] = synth->Generate(inc);
	}

	bool silent = false;
	return silent?FMOD_ERR_DSP_SILENCE:FMOD_OK;
}