#ifndef VOI_H
#define VOI_H

#include "common.h"
#include "data.h"

struct SDL_Window;

bool InitGL(SDL_Window*);
void DeinitGL();

struct SceneData;

void InitScene(Scene*, const SceneData*);
void RenderMesh(Scene*, u16 meshID, vec3, quat, vec3 = vec3{1,1,1});
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask);

ShaderProgram InitShaderProgram(const char*, const char*);

Framebuffer InitFramebuffer(u32, u32);
void DrawFullscreenQuad();

void InitParticleSystem(ParticleSystem*, u32);
void UpdateParticleSystem(ParticleSystem*, f32);
void RenderParticleSystem(ParticleSystem*);
void EmitParticles(ParticleSystem*, u32, f32, vec3);

#endif