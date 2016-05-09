#ifndef VOI_H
#define VOI_H

#include "common.h"
#include "data.h"

struct SDL_Window;

bool InitGL(SDL_Window*);
void DeinitGL();

bool InitDebugDraw();
void DrawDebug(const mat4& viewProjection);

void DebugLine(const vec3& a, const vec3& b, const vec3& col = vec3{1});
void DebugLine(const vec3& a, const vec3& b, const vec3& cola, const vec3& colb);

void DebugPoint(const vec3& pos, const vec3& col = vec3{1});

struct SceneData; // In sceneloader.h

bool InitScene(Scene*, const SceneData*);
void RenderMesh(Scene*, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale = vec3{1,1,1});
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask);

ShaderProgram CreateShaderProgram(const char* vs, const char* fs);

Framebuffer CreateFramebuffer(u32 width, u32 height);
void DrawFullscreenQuad();

bool InitParticleSystem(ParticleSystem*, u32 maxParticles);
void UpdateParticleSystem(ParticleSystem*, f32 dt);
void RenderParticleSystem(ParticleSystem*);
void EmitParticles(ParticleSystem*, u32 count, f32 lifetime, const vec3& pos);

bool InitPhysics(PhysicsContext*);
void UpdatePhysics(PhysicsContext*, Scene*, f32 dt);

#endif