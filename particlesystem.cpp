#include "voi.h"

void InitParticleSystem(ParticleSystem* sys, u32 numParticles) {
	sys->freeIndex = 0;
	sys->numParticles = numParticles;
	sys->positions = new vec3[numParticles];
	sys->velocities = new vec3[numParticles];
	sys->accelerations = new vec3[numParticles];
	sys->lifetimes = new f32[numParticles];
	sys->lifeRates = new f32[numParticles];

	for(u32 i = 0; i < numParticles; i++) {
		sys->lifetimes[i] = -1.f;
	}

	glGenBuffers(1, &sys->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sys->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numParticles * sizeof(f32) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UpdateParticleSystem(ParticleSystem* sys, f32 dt) {
	for(u32 i = 0; i < sys->numParticles; i++) {
		if(sys->lifetimes[i] < 0.f) continue;
		sys->lifetimes[i] -= sys->lifeRates[i] * dt;

		sys->positions[i] += sys->velocities[i] * dt;
		sys->velocities[i] += sys->accelerations[i] * dt;
	}
}

void RenderParticleSystem(ParticleSystem* sys) {
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, sys->vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sys->numParticles * sizeof(vec3), sys->positions);
	glBufferSubData(GL_ARRAY_BUFFER, sys->numParticles * sizeof(vec3), sys->numParticles * sizeof(f32), sys->lifetimes);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, (void*)((u64)sys->numParticles * sizeof(vec3)));

	glDrawArrays(GL_POINTS, 0, sys->numParticles);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(1);
}

static f32 randf(f32 a = -1, f32 b = 1) {
	return glm::linearRand<f32>(a,b);
}

void EmitParticles(ParticleSystem* sys, u32 count, f32 lifetime, vec3 position) {
	u32 i = sys->freeIndex;
	while(count > 0) {
		do {
			sys->positions[sys->freeIndex] = position + vec3{randf(-15, 15), randf(-6, 6), randf(-15, 15)};
			sys->velocities[sys->freeIndex] = glm::ballRand(0.05f);
			sys->accelerations[sys->freeIndex] = glm::normalize(vec3{randf(),randf()-0.5f,randf()})*0.03f;
			sys->lifetimes[sys->freeIndex] = 1.f;
			sys->lifeRates[sys->freeIndex] = 1.f/lifetime;
		} while(i++ < sys->numParticles && --count);
		
		if(i >= sys->numParticles) {
			i = 0;
		}
	}
	sys->freeIndex = i;
}