#ifndef TESTAUDIOGENERATORS_H
#define TESTAUDIOGENERATORS_H

#include "audiogenerator.h"

struct DoorAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		auto bar2 = std::fmod(elapsed/2.0*0.8, 1.0);
		auto bar8 = std::fmod(elapsed/8.0*0.8, 1.0);
		auto A = ntof(128);

		auto o = Wave::Triangle(elapsed * A * (bar2>0.1? 5.0/4.0 : 2.0)) * 2.f;
		if(bar8 > 0.5){
			o += Wave::Triangle(elapsed * A * 3./2.);
		}else{
			o += Wave::Triangle(elapsed * A * 3./4.);
		}

		o += Wave::Saw(elapsed * A / 2.0) * 0.2f;
		o += Wave::Saw(elapsed * (A+0.1) / 2.0) * 0.2f;
		o += Wave::Noise() * 0.3f;

		return o;
	}
};

struct TrophyAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		auto bar2 = std::fmod(elapsed/2.0*0.8, 1.0);
		auto bar8 = std::fmod(elapsed/8.0*0.8, 1.0);
		auto A = ntof(128);
		f32 o = 0.f;

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

		return o;
	}
};

struct FourWayAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		auto bar8 = std::fmod(elapsed/8.0*0.8, 1.0);
		f32 o = 0.f;

		if(bar8 < 0.5){
			o += Wave::Triangle(elapsed * ntof(128));
			o += Wave::Sin(elapsed * ntof(128+4));
		}else{
			o += Wave::Triangle(elapsed * ntof(128+4));
			o += Wave::Sin(elapsed * ntof(128+9));
		}
		o += Wave::Noise() * 0.35f * Wave::Sin(Wave::Sin(elapsed*0.05f)*0.5+0.5);

		return o;
	}
};

struct HighArpeggiatorAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		switch(((s32)(elapsed*6.))%6){
		case 0: return Wave::Sin(elapsed * ntof(140));
		case 1: return Wave::Sin(elapsed * ntof(144));
		case 2: return Wave::Saw(elapsed * ntof(147));
		case 3: return Wave::Sin(elapsed * ntof(149));
		case 4: return Wave::Sin(elapsed * ntof(152));
		case 5: return Wave::Saw(elapsed * ntof(154));
		default: return 0.f;
		}
	}
};

struct LowArpeggiatorAudioGenerator : AudioGenerator {
	f32 Generate(f64 elapsed) override {
		switch(((s32)(elapsed*4.))%5){
		case 0: return Wave::Sin(elapsed * ntof(128))*2.0f;
		case 1: return Wave::Sin(elapsed * ntof(132))*2.0f;
		case 2: return Wave::Sin(elapsed * ntof(135))*2.0f;
		case 3: return Wave::Sin(elapsed * ntof(137))*2.0f;
		case 4: return Wave::Triangle(elapsed * ntof(139))*2.0f;
		default: return 0.f;
		}
	}
};

struct NoiseAudioGenerator : AudioGenerator {
	f32 Generate(f64) override {
		return floor(Wave::Noise()*2.f)/2.f;
	}
};

#endif