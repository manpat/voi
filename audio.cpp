#include "voi.h"
#include "lua-synth/synth.h"
#include <map>

namespace {
	std::map<u32, u32> synthToEntity;
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

void UpdateAudio(Scene* scene) {
	extern Entity* playerEntity;
	if(!playerEntity) return;

	const auto right = playerEntity->rotation*vec3{1, 0, 0};
	const auto fwd = playerEntity->rotation*vec3{0, 0,-1};
	
	for(auto se: synthToEntity) {
		auto s = synth::GetSynth(se.first);
		if(!s) continue;

		auto e = GetEntity(se.second);
		if(!e) continue;

		if(playerEntity->layers & e->layers) {
			// They occupy the same layers, so proceed normally
			const auto diff = e->position - playerEntity->position;
			const auto dir = glm::normalize(diff);
			const f32 dist = glm::length(diff);

			const auto rightness = glm::dot(dir, right);
			const auto behindness = glm::clamp(-glm::dot(dir, fwd), 0.f, 1.f);

			// TODO: Make this configurable
			// auto gain = glm::clamp(60.f/(dist+1), 0.f, 1.f);
			auto gain = glm::clamp(1-dist/60.f, 0.f, 1.f);
			gain *= gain;

			SetSynthPan(s, rightness * 0.5f);
			SetSynthGain(s, (1.f - behindness*behindness*0.5) * gain);
		}else{
			// Find a portal that connects the player and audio source
			// NOTE: This only goes one layer deep
			// TODO: Find a way to average several nearby portals, instead of just using nearest

			f32 bestCompoundDistance = std::numeric_limits<f32>::max();
			vec3 bestPlayerDiff{fwd};

			for(u16 i = 0; i < scene->numPortals; i++) {
				const auto pIdx = scene->portals[i];
				const auto portal = &scene->entities[pIdx];
				if(portal->entityType != Entity::TypePortal)
					continue;

				const u32 layers = portal->layers;
				if(!(layers&playerEntity->layers) || !(layers&e->layers))
					continue;

				const auto playerDiff = portal->position - playerEntity->position;
				const auto audioDiff = portal->position - e->position;
				const auto compoundDist = glm::length(playerDiff) + glm::length(audioDiff); 

				if(compoundDist < bestCompoundDistance) {
					bestCompoundDistance = compoundDist;
					bestPlayerDiff = playerDiff;
				}
			}

			const auto dir = glm::normalize(bestPlayerDiff);

			const auto rightness = glm::dot(dir, right);
			const auto behindness = glm::clamp(-glm::dot(dir, fwd), 0.f, 1.f);

			// TODO: Make this configurable
			// auto gain = glm::clamp(60.f/(dist+1), 0.f, 1.f);
			auto gain = glm::clamp(1-bestCompoundDistance/60.f, 0.f, 1.f);
			gain *= gain;

			SetSynthPan(s, rightness * 0.5f);
			SetSynthGain(s, (1.f - behindness*behindness*0.5) * gain);
		}
	}
}

void AttachSynthToEntity(u32 entID, u32 synthID) {
	synthToEntity[synthID] = entID;
}