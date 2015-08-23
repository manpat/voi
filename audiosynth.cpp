#include "audiosynth.h"

#include "audiomanager.h"

// AUDIOSYNTH

AudioSynth::AudioSynth() {
	//auto fmod = AudioManager().getSystem();
	scheduler = new NoteScheduler();
}

f64 ntof(u8 n) {
	return 220.0 * std::pow(2.0, ((s32)n - 128) / 12.0);
}

f32 AudioSynth::Generate(f64 dt) {
	if (!scheduler)
	{
		return 0.0f;
	}
	scheduler->Update(phase);

	f32 amp = 0.0f;
	f32 o = 0.0;
	scheduler->ForEachActive([&](Note& ni) {
		auto freq = ntof(ni.note);
		auto envamp = env.Generate(phase, ni);

		amp += envamp;

		o += osc.Generate(phase, freq) * envamp;
	});

	phase += dt;

	o *= 0.5;
	return o;
}

// ENVELOPE

f32 AudioSynth::Envelope::Generate(f32 phase, Note& note) {
	phase *= 90.0 / 60.0;
	auto position = phase - note.beginTime;

	auto end = note.beginTime + attack + decay;
	if (end < note.endTime) end = note.endTime;

	auto rpos = phase - end;

	if (position <= attack) {
		return std::min(position / attack, 1.f);

	}
	else if (position - attack <= decay) {
		return std::max(1.0 - (position - attack) / decay *(1.0 - sustain), 0.0);

	}
	else if (phase < note.endTime) {
		return sustain;

	}
	else if (rpos < release) {
		return std::max((1.0 - rpos / release)*sustain, 0.0);
	}

	return 0.0;
}

// OSCILLATOR

f32 AudioSynth::Oscillator::Generate(f64 phase, f64 frequency) {
	switch (waveform) {
	case waveformType::Sine:
		return std::sin(M_PI*2.0*(frequency * detune * octave)*phase);

	case waveformType::Square:
		return (std::fmod(phase*(frequency * detune * octave), 1.0) < pulsewidth) ? -1.0 : 1.0;

	case waveformType::Saw:
		return fmod(phase*(frequency * detune * octave)*2.0, 2.0) - 1.0;

	case waveformType::Triangle: {
		auto nph = fmod(phase*(frequency * detune * octave), 1.0);
		if (nph <= 0.5) return (nph - 0.25)*4.0;

		return (0.75 - nph)*4.0;
	}
	default: break;
	}

	return 0.0f;
}

// NOTE SCHEDULER

void AudioSynth::NoteScheduler::Update(f32 t) {
	time = t * 90 / 60.0;

	auto end = notes.end();
	auto nend = std::remove_if(notes.begin(), end, [](const Note& n) {
		return n.dead();
	});

	notes.erase(nend, end); // This crashes the server when the client crashes (think it's only when server is in debug mode)
}

void AudioSynth::NoteScheduler::NoteOn(u8 note, f32 offset) {
	if (note == 0) throw "NoteOn signal for note 0 is invalid";
	f32 begin = time + offset;

	notes.push_back(Note{ begin, -1.0f, note, 0x3 /* Two envelopes */ });
}

void AudioSynth::NoteScheduler::NoteOff(u8 note) {
	if (notes.size()) {
		auto ninfo = std::find_if(notes.begin(), notes.end(), [note](const Note& info) {
			// Will search for a note of the same value and that is held
			return info.note == note;
		});
		if (ninfo == notes.end()) return;

		f32 end = time;

		ninfo->endTime = end;
	}
}

void AudioSynth::NoteScheduler::Clear() {
	notes.clear();
}

void AudioSynth::NoteScheduler::ForEachActive(std::function<void(Note&)> func) {
	for (auto& ni : notes) {
		if (ni.beginTime > time) continue;
		if (ni.dead()) continue;

		func(ni);
	}
}

// FMOD

FMOD_RESULT F_CALLBACK AudioSynth::GeneratorFunction(FMOD_DSP_STATE* state, f32*, f32* outbuffer, u32 length, s32, s32*) {
	s32 samplerate = 0;
	cfmod(state->callbacks->getsamplerate(state, &samplerate));
	f64 inc = 1.0 / samplerate;

	FMOD::DSP *thisdsp = (FMOD::DSP *)state->instance;

	void* ud = nullptr;
	cfmod(thisdsp->getUserData(&ud));
	auto& synth = *static_cast<AudioSynth*>(ud);

	for (u32 i = 0; i < length; ++i) {
		outbuffer[i] = synth.Generate(inc);
	}

	return FMOD_OK;
}