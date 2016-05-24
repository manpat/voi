#ifndef DATA_H
#define DATA_H

#include "common.h"
#include <vector>

class btRigidBody;
class btCollisionShape;
class btDbvtBroadphase;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
struct Entity;
struct Scene;

struct Mesh {
	enum { MaxInlineSubmeshes = 4 };

	static constexpr u32 ElementTypeToGL[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};

	u32 numTriangles = 0;
	u32 vbo = 0;
	u32 ebo = 0;
	u8 elementType = 0; // 0,1,2

	struct Submesh {
		u32 triangleCount;
		u8 materialID;
		// 3B free here
	};

	u8 numSubmeshes = 0;
	// TODO: This could be simplified by having a single external submesh pool
	union {
		// Save allocations for meshes with few materials
		Submesh submeshesInline[MaxInlineSubmeshes];
		Submesh* submeshes;
	};

	// Stuff for physics
	vec3 extents;
	vec3 center;
};

struct Material {
	const char* name;
	vec3 color;
	u32 flags = 0;
	u32 shaderID = 0;
};

struct EntityManager {
	enum {
		FreeEntityBucketSize = 512,
		FreeEntityIDOffset = 0x8000 // ~32k
	};

	struct Bucket {
		Entity* entities;
		u16 capacity;
		u16 used;
		u8 id;
	};

	std::vector<Bucket> entityBuckets;
	Bucket sceneEntityBuckets[2];
};

enum ColliderType {
	ColliderNone,
	ColliderCube,
	ColliderCylinder,
	ColliderCapsule,
	ColliderConvex,
	ColliderMesh,
};

struct PhysicsColliderPair {
	u16 entityID0;
	u16 entityID1;
	u32 stamp;
};

struct RaycastResult {
	Entity* entity;
	vec3 hitPosition;
	vec3 hitNormal;
	f32 distance;
	
	bool hit() const { return entity != nullptr; }
};

struct PhysicsContext {
	btDbvtBroadphase* broadphase;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* world;

	std::vector<PhysicsColliderPair> activeColliderPairs;
	bool needsRefilter = false;
	u32 currentStamp = 0;
};

struct Entity {
	enum : u8 {
		TypeGeometry,
		TypePortal,
		TypeMirror,
		TypeTrigger,

		// NOTE: Nothing from this point onwards
		//	is exposed in the editor and should
		//	only be created in code.
		TypeNonExportable = 128,
		TypePlayer = TypeNonExportable,
	};

	enum : u8 {
		FlagHidden		= 1<<0,
		FlagStatic		= 1<<1,
		FlagInteractive = 1<<2,
	};

	u16 id;
	u32 flags;

	u32 layers;
	vec3 position;
	quat rotation;
	vec3 scale;

	u16 parentID;
	u16 meshID;

	u8 nameLength;
	const char* name;

	// u16 scriptID; // TODO
	u8 entityType; // Type*
	u8 colliderType;

	btRigidBody* rigidbody;
	btCollisionShape* collider;

	// When ownedByScene is true, scene points to the owning scene
	//	otherwise it points to the scene that the entity is currently in
	Scene* scene;
	bool ownedByScene; 

	vec3 extents;
	vec3 centerOffset;

	// NOTE: Assume that nothing in this union is initialised
	union {
		// Valid with TypePortal and TypeMirror
		vec3 planeNormal;

		struct {
			vec3 eyeOffset;
			vec2 mouseRot;
		} player;
	};

	// So I can use the union for things with non-trivial constructors (glm vectors)
	Entity() {}
};

struct ShaderProgram {
	u32 program;

	u32 depthTexLoc = 0;
	u32 colorTexLoc = 0;
	u32 general0TexLoc = 0;
	u32 general1TexLoc = 0;
	
	u32 materialColorLoc = 0;
	u32 clipPlaneLoc = 0;

	u32 viewProjectionLoc = 0;
	u32 modelLoc = 0;
};

enum {
	ShaderIDDefault,
	ShaderIDParticles,
	ShaderIDPost,
	ShaderIDUI,
	ShaderIDCount,
	ShaderIDCustom = ShaderIDCount,
};

struct Scene {
	u16 numMeshes = 0;
	u16 numEntities = 0;

	u32 nameArenaSize = 0;
	char* nameArena = nullptr;
	char* nameArenaFree = nullptr;

	Entity* entities;
	Mesh* meshes;
	Material materials[256];
	ShaderProgram shaders[256];

	u32 portals[256];
	u16 numPortals;

	PhysicsContext physicsContext;
};

struct Camera {
	mat4 projection;
	
	vec3 position;
	quat rotation;
};

enum {
	FBTargetDepthStencil,
	FBTargetColor,
	FBTargetGeneral0,
	FBTargetGeneral1,
	FBTargetCount
};

struct Framebuffer {
	u32 fbo;
	u32 targets[FBTargetCount];
	bool valid;
};

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

#endif