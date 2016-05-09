#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "common.h"

struct ParticleSystem {
	u32 numParticles;
	u32 freeIndex;
	vec3* positions;
	vec3* velocities;
	vec3* accelerations;
	f32* lifetimes;
	f32* lifeRates;

	u32 vertexBuffer;
};

void InitParticleSystem(ParticleSystem*, u32);
void UpdateParticleSystem(ParticleSystem*, f32);
void RenderParticleSystem(ParticleSystem*);
void EmitParticles(ParticleSystem*, u32, f32, vec3);

#endif