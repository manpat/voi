#include "entity.h"
#include "audiomanager.h"
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
		cfmod(channel->set3DMinMaxDistance(3.0f, 10000.0f));
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

// ------ TEMPORARY ------
namespace Wave {
	f32 sin(f64 phase){
		return (f32)std::sin(2.0*M_PI*phase);
	}

	f32 saw(f64 phase){
		return (f32)(fmod(phase, 2.0)-1.0);
	}

	f32 sqr(f64 phase, f64 width = 0.5){
		auto nph = fmod(phase, 1.0);
		if(nph < width) return -1.0;

		return 1.0;
	}

	f32 tri(f64 phase){
		auto nph = fmod(phase, 1.0);
		if(nph <= 0.5) return (f32)((nph-0.25)*4.0);

		return (f32)((0.75-nph)*4.0);
	}
}
// ------ TEMPORARY ------

f32 SynthComponent::Generate(f64 dt) {
	f32 o = 0.f;
	auto bar2 = std::fmod(elapsed/2.0, 1.0);
	auto bar8 = std::fmod(elapsed/8.0, 1.0);

	auto A = ntof(128);

	switch(mode){
	case 0:
		o += (f32)(Wave::tri(elapsed * A * (bar2>0.1? 5.0/4.0 : 2.0)) * 2.0);
		o += (f32)Wave::tri(elapsed * A * 3./2.);
		o += (f32)(Wave::saw(elapsed * A / 2.0) * 0.333);
		o += (f32)(Wave::saw(elapsed * (A+0.1) / 2.0) * 0.333);
		break;

	case 1:
		if(bar2 > 0.9){
			o += (f32)(Wave::tri(elapsed * A * 2.0) * 3.0 * (bar2 - 0.9)/0.1);
		}else{
			o += Wave::sqr(elapsed * A * 5.0/4.0, 0.3);
		}
		o += Wave::tri(elapsed * A / 2.0);
		break;

	case 2:
		if(bar8 < 0.5){
			o += Wave::tri(elapsed * ntof(128));
			o += Wave::sin(elapsed * ntof(128+4));
		}else{
			o += Wave::tri(elapsed * ntof(128+4));
			o += Wave::sin(elapsed * ntof(128+9));
		}
		break;

	case 3:
		switch(((s32)(elapsed*7.))%5){
		case 0: o += Wave::sin(elapsed * ntof(140));break;
		case 1: o += Wave::sin(elapsed * ntof(144));break;
		case 2: o += Wave::sin(elapsed * ntof(147));break;
		case 3: o += Wave::sin(elapsed * ntof(149));break;
		case 4: o += Wave::sin(elapsed * ntof(152));break;
		}

	case 4:
		switch(((s32)(elapsed*3.))%5){
		case 0: o += Wave::sin(elapsed * ntof(128));break;
		case 1: o += Wave::sin(elapsed * ntof(132));break;
		case 2: o += Wave::sin(elapsed * ntof(135));break;
		case 3: o += Wave::sin(elapsed * ntof(137));break;
		case 4: o += Wave::sin(elapsed * ntof(139));break;
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