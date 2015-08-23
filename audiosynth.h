#ifndef AUDIOSYNTH_H
#define AUDIOSYNTH_H

#include <fmod.hpp>

#include "common.h"

class AudioSynth {
public:
	AudioSynth();

	f32 Generate(f64);

	FMOD::Channel* channel;
	FMOD::DSP* dsp;

	f64 phase = 0;

	struct Note {
		f32 beginTime;
		f32 endTime;
		u8 note;
		u8 envFlags;

		bool dead() const { return envFlags == 0; }
	};

private:
	struct Envelope {
		f32 attack = 100;
		f32 decay = 100;
		f32 sustain = 100;
		f32 release = 100;

		f32 Generate(f32 phase, AudioSynth::Note& note);
	};

	struct Oscillator {
		enum waveformType
		{
			Sine,
			Square,
			Triangle,
			Saw
		};
		waveformType waveform = waveformType::Saw;
		f32 octave = 1.0;
		f32 detune = 1.0;
		f32 pulsewidth = 0.5;

		f32 Generate(f64 phase, f64 frequency);
	};

	Envelope env;
	Oscillator osc;

	static FMOD_RESULT F_CALLBACK GeneratorFunction(FMOD_DSP_STATE*, f32*, f32*, u32, s32, s32*);

public:
	struct NoteScheduler {
		std::vector<Note> notes;
		f32 time = 0.0;

		void Update(f32 dt);

		// note is in semitones
		// 128 is A 220
		void NoteOn(u8, f32 = 0.0);
		void NoteOff(u8);
		void Clear();

		void ForEachActive(std::function<void(Note&)>);
	};

	NoteScheduler* scheduler;
};



#endif//AUDIOSYNTH_H