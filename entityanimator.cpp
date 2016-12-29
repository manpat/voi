#include "voi.h"
#include <vector>
#include <algorithm>

struct AnimationCommand {
	enum {
		TypeNone,
		TypePosition,
	};

	u8 type;

	u16 entityID;
	f32 duration;
	f32 progress;

	union {
		struct {
			vec3 original;
			vec3 target;
		} position;
	};

	AnimationCommand() {}
};

namespace {
	std::vector<AnimationCommand> animationCmds;
}

template<class T>
static T Lerp(T initial, T target, f32 x) {
	f32 term = x*x*(3.f - 2.f*x); // Smoothstep
	return glm::mix(initial, target, term);
}

bool InitEntityAnimator() {
	return true;
}

void UpdateEntityAnimator(f32 dt) {
	RemoveFromVectorIf(&animationCmds, [](const AnimationCommand& cmd) {
		return (cmd.progress > 1.f) || !cmd.entityID;
	});

	for(auto& cmd: animationCmds) {
		auto e = GetEntity(cmd.entityID);
		if(!e) {
			cmd.entityID = 0;
			continue;
		}

		cmd.progress = glm::clamp(cmd.progress + dt/cmd.duration, 0.f, 1.f);

		switch(cmd.type) {
			case AnimationCommand::TypePosition: {
				auto cpos = &cmd.position;
				auto npos = Lerp(cpos->original, cpos->target, cmd.progress);
				e->position = npos;
				WakeUpEntity(e);
			}	break;

			case AnimationCommand::TypeNone:
			default: break;
		}
	}
}

void QueueEntityMoveToAnimation(Entity* e, vec3 target, f32 duration) {
	if(!e) {
		LogError("Warning! Tried to animate null entity!\n");
		return;
	}

	AnimationCommand cmd {};
	cmd.type = AnimationCommand::TypePosition;
	cmd.entityID = e->id;

	cmd.duration = glm::max(duration, 0.001f);
	cmd.progress = 0.f;

	cmd.position = {e->position, target};
	animationCmds.push_back(cmd);

	SetEntityKinematic(e, true, false);
}