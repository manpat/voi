#include "voi.h"
#include "lua-synth/synth.h"
#include <map>

namespace {
	std::map<u32, u32> synthToEntity;
	std::map<u32, vec2> synthToParams;
}

bool InitAudio() {
	if(!synth::InitAudio()) {
		LogError("Synth init failed");
		return false;
	}

	return true;
}

void DeinitAudio() {
	synth::DeinitAudio();
}

void UpdateAudio() {
	extern Entity* playerEntity;
	if(!playerEntity) return;
	
	for(auto se: synthToEntity) {
		auto s = synth::GetSynth(se.first);
		if(!s) continue;

		auto e = GetEntity(se.second);
		if(!e) continue;

		// const auto ear1 = playerEntity->position + playerEntity->rotation*vec3{-0.5, 0, 0};
		// const auto ear2 = playerEntity->position + playerEntity->rotation*vec3{ 0.5, 0, 0};

		// const auto diff1 = e->position - ear1;
		// const auto diff2 = e->position - ear2;

		const auto diff = e->position - playerEntity->position;
		const auto dir = glm::normalize(diff);
		const f32 dist = glm::length(diff);

		const auto right = playerEntity->rotation*vec3{1, 0, 0};
		const auto fwd = playerEntity->rotation*vec3{0, 0,-1};

		const auto rightness = glm::dot(dir, right);
		const auto behindness = glm::clamp(-glm::dot(dir, fwd), 0.f, 1.f);

		const auto gain = glm::clamp(100.f/(dist+1), 0.f, 1.f);

		SetSynthPan(s, rightness * 0.5f);
		SetSynthGain(s, (1.f - behindness*behindness*0.5) * gain);
	}
}

void AttachSynthToEntity(u32 entID, u32 synthID) {
	synthToEntity[synthID] = entID;
}