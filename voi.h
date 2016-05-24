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
void UpdatePhysics(Scene*, f32 dt);

RaycastResult Raycast(Scene*, const vec3&, const vec3&, f32 = std::numeric_limits<f32>::infinity(), u32 layermask = ~0u);
RaycastResult Linecast(Scene*, const vec3&, const vec3&, u32 layermask = ~0u);

// NOTE: Using MeshData here is kinda dirty and
//	ties physics init to scene loading but we
//	don't keep the data around long enough to 
//	decouple them
struct MeshData;
bool InitEntityPhysics(Entity*, const MeshData*);
void DeinitEntityPhysics(Entity*);

void SetEntityRotation(Entity*, const quat&);
void SetEntityVelocity(Entity*, const vec3&);
vec3 GetEntityVelocity(const Entity*);
vec3 GetEntityCenterOfMass(const Entity*);
void ConstrainEntityUpright(Entity*);

void UpdateEntity(Entity*, f32 dt);
void EntityOnCollisionEnter(Entity*, Entity*);
void EntityOnCollisionLeave(Entity*, Entity*);
void EntityOnTriggerEnter(Entity*, Entity*);
void EntityOnTriggerLeave(Entity*, Entity*);

Entity* AllocateSceneEntities(u16 count);
Entity* AllocateEntity();
void FreeSceneEntities(Entity* se);
void FreeEntity(Entity* e);
Entity* GetEntity(u16 id);

#endif