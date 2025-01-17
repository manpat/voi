#ifndef VOI_H
#define VOI_H

#include "common.h"
#include "data.h"
#include "voigl.h"
#include <initializer_list>

struct SDL_Window;

void ParseCLOptions(s32, char**);
void LoadOptions();

bool GetBoolOption(const char*);
s64 GetIntOption(const char*);
f64 GetFloatOption(const char*);
const char* GetStringOption(const char*);

// Enables warnings for format params on GCC
#ifdef __GNUC__
#define FORMATARGS(f,s) __attribute__((format(printf, (f), (s))))
#else
#define FORMATARGS(f,s)
#endif

bool InitLog();
void Log(const char*, ...) FORMATARGS(1,2);
void LogError(const char*, ...) FORMATARGS(1,2);

#undef FORMATARGS

FileData LoadFile(const char* filename, bool nullTerminate);
char* LoadFileStatically(const char* filename, bool nullTerminate = true);
void CleanupStaticallyLoadedFiles();

bool InitGL(SDL_Window*);
void DeinitGL();

bool InitDebug();
void DrawDebug(const mat4& viewProjection);

void DebugLine(const vec3& a, const vec3& b, const vec3& col = vec3{1});
void DebugLine(const vec3& a, const vec3& b, const vec3& cola, const vec3& colb);
void DebugPoint(const vec3& pos, const vec3& col = vec3{1});

bool InitAudio();
void DeinitAudio();
void UpdateAudio(Scene*);
void AttachSynthToEntity(u32 entID, u32 synthID);
void SetSynthFalloffMode(u32 synthID, SynthFalloffMode falloffMode);
void SetSynthFalloffDistance(u32 synthID, f32);

struct SceneData; // In sceneloader.h

bool InitScene(Scene*, const SceneData*);
void DeinitScene(Scene*);
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask);

ShaderProgram CreateShaderProgram(const char* vs, const char* fs);
ShaderProgram* CreateNamedShaderProgram(u32 shId, const char* vs, const char* fs);
ShaderProgram* GetNamedShaderProgram(u32 shId);
void UseShaderProgram(const ShaderProgram*);

Framebuffer CreateFramebuffer(FramebufferSettings);
void EnableTargets(std::initializer_list<u32>);
void EnableTargets(u32, const u32* const);

void DestroyFramebuffer(Framebuffer*);
u32 LoadTexture(const char* fname);
void DrawUnitQuad();
void DrawQuadAtFarPlane(const mat4& projection);

bool InitEffects();
bool ReinitEffects();
void ApplyEffectsAndDraw(Framebuffer*, const Camera*, f32 dt);
void SetTargetFogParameters(vec3 color, f32 distance, f32 density, f32 duration);
void SetTargetFogColor(vec3, f32 duration = 4.f);
void SetTargetFogDistance(f32, f32 duration = 4.f);
void SetTargetFogDensity(f32, f32 duration = 4.f);
void SetTargetVignetteLevel(f32, f32 duration = 4.f);

vec3 RGBToHSV(vec3);
vec3 HSVToRGB(vec3);

bool InitScripting();
s32 LoadScript(const char* fname);
s32 GetCallbackFromScript(s32 script, const char* funcName);
void PushCallbackParameter(std::nullptr_t);
void PushCallbackParameter(u32);
void PushCallbackParameter(s32);
void PushCallbackParameter(f32);
void PushCallbackParameter(bool);
void PushCallbackParameter(const char*);
void RunCallback(s32 func);
void UnloadScript(s32 script);

bool InitParticleSystem(ParticleSystem*, u32 maxParticles);
void DeinitParticleSystem(ParticleSystem*);
void UpdateParticleSystem(ParticleSystem*, f32 dt);
void RenderParticleSystem(ParticleSystem*);
void EmitParticles(ParticleSystem*, u32 count, f32 lifetime, const vec3& pos);

bool InitPhysics(PhysicsContext*);
void DeinitPhysics(PhysicsContext*);
void UpdatePhysics(Scene*, f32 dt);
void RefilterEntity(Entity*);

RaycastResult Raycast(Scene*, const vec3&, const vec3&, f32 = constant::infinity, u32 layermask = ~0u);
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
void SetEntityKinematic(Entity*, bool, bool = true);
void WakeUpEntity(Entity*);

void InitEntity(Entity*);
void DeinitEntity(Entity*);
void UpdateEntity(Entity*, f32 dt);
void UpdateEntityPhysicsRate(Entity*, f32 dt);
void EntityOnCollisionEnter(Entity*, Entity*);
void EntityOnCollisionLeave(Entity*, Entity*);

const char* GetEntityTypeName(u8);

Entity* AllocateSceneEntities(u16 count);
Entity* AllocateEntity();
void FreeSceneEntities(Entity* se);
void FreeEntity(Entity* e);
Entity* GetEntity(u16 id);
Entity* FindEntity(const char* name);
EntityIterator GetEntityIterator();

bool InitEntityAnimator();
void UpdateEntityAnimator(f32 dt);
void QueueEntityMoveToAnimation(Entity*, vec3 target, f32 duration);

// Helpers

#include <vector>

template<class T, class Func>
void RemoveFromVectorIf(std::vector<T>*, Func&&);

template<class T, u64 size>
constexpr u64 GetArraySize(const T (&)[size]);

#include "helpers.inl"

#endif