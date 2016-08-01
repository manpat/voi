#ifndef DATA_H
#define DATA_H

#include "common.h"
#include "voigl.h"
#include <vector>

class btRigidBody;
class btCollisionShape;
struct btDbvtBroadphase;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
struct Camera;
struct Entity;
struct Scene;

struct Mesh {
	enum { MaxInlineSubmeshes = 4 };

	u32 numTriangles = 0;
	u32 indexOffset = 0;

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
	u32 currentStamp = 0;
};

struct Entity {
	enum : u8 {
		TypeGeometry,
		TypePortal,
		TypeMirror,
		TypeInteractive,
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
		FlagIgnoreFog	= 1<<2,
	};

	u16 id;
	u32 flags;

	u32 layers;
	vec3 position;
	quat rotation;
	vec3 scale;

	u16 parentID; // NOTE: Unused
	u16 meshID;

	u8 nameLength;
	const char* name;

	u8 entityType; // Type*
	u8 colliderType;

	s32 updateCallback;

	btRigidBody* rigidbody;
	btCollisionShape* collider;

	// When ownedByScene is true, scene points to the owning scene
	//	otherwise it points to the scene that the entity is currently in
	Scene* scene;
	bool ownedByScene; 

	vec3 extents;
	vec3 centerOffset;

	// NOTE: By default, everything here is initialised to zero
	union {
		vec3 planeNormal;

		struct {
			vec3 eyeOffset;
			vec2 mouseRot;

			f32 slopeSpeedAdjustSmooth;
			f32 slopeJumpAdjustSmooth;
			f32 jumpTimeout;
			bool canJump;

			u32 originalLayers;
			u16 collidingPortalID;
			s8 portalSide;

			bool lookingAtInteractive;

			// TODO: Change this, I don't like it
			Camera* camera;
		} player;

		struct {
			s32 frobCallback;
		} interact;

		struct {
			s32 enterCallback;
			s32 leaveCallback;
		} trigger;
	};

	// So I can use the union for things with non-trivial constructors (glm vectors)
	Entity() {}
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
	// NOTE: sceneEntityBuckets[1] is for temp storage and won't be updated
};

struct EntityIterator {
	s32 bucketID; // <0 is scene entity bucket
	s32 entityOffset; // <0 is invalid

	static EntityIterator begin();
	static EntityIterator end();
	EntityIterator& operator++();
	EntityIterator operator++(int);
	bool operator!=(const EntityIterator&);
	bool operator==(const EntityIterator&);
	Entity* operator*();
	Entity* operator->();
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
	u32 projectionLoc = 0;
	u32 viewLoc = 0;
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
	vec3 materials[256];
	ShaderProgram shaders[256];

	u32 portals[256];
	u16 numPortals;

	PhysicsContext physicsContext;
};

struct Camera {
	mat4 projection;
	mat4 view;
	
	vec3 position;
	quat rotation;

	f32 nearDist;
	f32 farDist;
	f32 aspect;
	f32 fov;

	u32 intersectingPortalId;
};

// NOTE: In OpenGL 3.x+ the minimum available number of color attachments and draw buffers is 8
// TODO: Check how many are available to us
enum {
	FBTargetDepthStencil,
	FBTargetColor,
	FBTargetGeneral0,
	FBTargetGeneral1, // NOTE: This is never initialised, probably do that before using it
	FBTargetCount
};

struct FramebufferSettings {
	u32 width, height;
	u32 numColorBuffers;
	bool hasStencil;
	bool hasDepth;
	bool filter;
};

struct Framebuffer {
	u32 fbo;
	u32 width, height;
	u32 depthStencilTarget;
	u32 colorTargets[8];
	u8 targetCount;
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