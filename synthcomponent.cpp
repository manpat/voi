#include "entity.h"
#include "audiomanager.h"
#include "audiogenerator.h"
#include "synthcomponent.h"

f64 ntof(u8 n) {
	return 220.0 * std::pow(2.0, ((s32)n - 128) / 12.0);
}

SynthComponent::SynthComponent(u32 _m) : Component(this), mode(_m) {}

void SynthComponent::OnInit() {
	auto audioMan = AudioManager::GetSingleton();

	FMOD_DSP_DESCRIPTION desc;
	memset(&desc, 0, sizeof(desc));

	desc.numinputbuffers = 0;
	desc.numoutputbuffers = 1;
	desc.read = GeneratorFunction;
	desc.userdata = this;

	cfmod(audioMan->system->createDSP(&desc, &dsp));
	cfmod(dsp->setChannelFormat(FMOD_CHANNELMASK_MONO, 1, FMOD_SPEAKERMODE_MONO));

	cfmod(audioMan->system->playDSP(dsp, audioMan->mastergroup, false, &channel));
	cfmod(channel->setMode(FMOD_3D));
	// TODO: Figure out values properly
	// e.g. test if entity has collider and use dimensions
	if(mode >= 3){
		cfmod(channel->set3DMinMaxDistance(0.4f, 10000.0f));
	}else{
		cfmod(channel->set3DMinMaxDistance(2.0f, 10000.0f));
	}

	auto newReverb = [&]() -> FMOD::DSP* {
		FMOD::DSP* rvb;
		cfmod(audioMan->system->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &rvb));
		cfmod(channel->addDSP(1, rvb));

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, -60.0);

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, 80.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, 10000.);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, 10.);

		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, 100.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, 100.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, 40.0);
		rvb->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, 1000.0);

		return rvb;
	};

	newReverb();
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
	f32 o = 0.f;
	auto speed = 0.8;
	auto bar2 = std::fmod(elapsed/2.0*speed, 1.0);
	auto bar8 = std::fmod(elapsed/8.0*speed, 1.0);

	auto A = ntof(128);

	switch(mode){
	case 0:
		o += Wave::Triangle(elapsed * A * (bar2>0.1? 5.0/4.0 : 2.0)) * 2.f;
		if(bar8 > 0.5){
			o += Wave::Triangle(elapsed * A * 3./2.);
		}else{
			o += Wave::Triangle(elapsed * A * 3./4.);
		}

		o += Wave::Saw(elapsed * A / 2.0) * 0.2f;
		o += Wave::Saw(elapsed * (A+0.1) / 2.0) * 0.2f;
		o += Wave::Noise() * 0.3f;
		break;

	case 1:
		if(bar2 < 0.1){
			o += Wave::Square(elapsed * A * 2.0, Wave::Noise() * 0.3 + 0.7) * 3.0f * (f32)bar2/0.1f;
		}else{
			o += Wave::Triangle(elapsed * A * 5.0/4.0);
		}
		
		o += Wave::Saw(elapsed * A / 2.0);
		if(bar8 > 0.5){
			o += Wave::Triangle(elapsed * A / 3.0);
		}else{
			o += Wave::Triangle(elapsed * A * 3.0 / 4.0);
		}
		break;

	case 2:
		if(bar8 < 0.5){
			o += Wave::Triangle(elapsed * ntof(128));
			o += Wave::Sin(elapsed * ntof(128+4));
		}else{
			o += Wave::Triangle(elapsed * ntof(128+4));
			o += Wave::Sin(elapsed * ntof(128+9));
		}
		o += Wave::Noise() * 0.35f * Wave::Sin(Wave::Sin(elapsed*0.05f)*0.5+0.5);

		break;

	case 3:
		switch(((s32)(elapsed*6.))%6){
		case 0: o += Wave::Sin(elapsed * ntof(140)); break;
		case 1: o += Wave::Sin(elapsed * ntof(144)); break;
		case 2: o += Wave::Saw(elapsed * ntof(147)); break;
		case 3: o += Wave::Sin(elapsed * ntof(149)); break;
		case 4: o += Wave::Sin(elapsed * ntof(152)); break;
		case 5: o += Wave::Saw(elapsed * ntof(154)); break;
		}

	case 4:
		switch(((s32)(elapsed*4.))%5){
		case 0: o += Wave::Sin(elapsed * ntof(128))*2.0f; break;
		case 1: o += Wave::Sin(elapsed * ntof(132))*2.0f; break;
		case 2: o += Wave::Sin(elapsed * ntof(135))*2.0f; break;
		case 3: o += Wave::Sin(elapsed * ntof(137))*2.0f; break;
		case 4: o += Wave::Triangle(elapsed * ntof(139))*2.0f; break;
		}

		break;

	default: break;
	}

	elapsed += dt;
	return o;
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