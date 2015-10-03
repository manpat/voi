#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include <vector>
#include <btBulletDynamicsCommon.h>
#include "common.h"
#include "singleton.h"
#include "component.h"

// Ray casting
// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Using_RayTest

struct ColliderComponent;

struct PhysicsManager : Singleton<PhysicsManager> {
	using Broadphase = btDbvtBroadphase;
	using Dispatcher = btCollisionDispatcher;
	using Solver = btSequentialImpulseConstraintSolver;
	using World = btDiscreteDynamicsWorld;

	struct ColliderPair {
		ColliderComponent* collider0;
		ColliderComponent* collider1;
		u8 stamp;
	};

	Broadphase* broadphase = nullptr;
	Dispatcher* dispatcher = nullptr;
	Solver* solver = nullptr;
	World* world = nullptr;

	// Stores pairs of triggers to colliding colliders and a timestamp
	//	to test for staleness. I'm not sure I like this implementation
	std::vector<ColliderPair*> activeColliderPairs = {};
	u8 currentStamp = 0;

	f32 timestep = 1.f/60.f;
	u32 enabledCollisionGroups = ~0u;

	PhysicsManager(f32 = 60.f);
	~PhysicsManager();

	void Update();

	void ProcessCollision(ColliderComponent*, ColliderComponent*);
	void ProcessTriggerCollision(ColliderComponent*, ColliderComponent*);
};

class EntityMotionState;

struct ColliderComponent : Component {
	using RigidBody = btRigidBody;
	using RigidBodyInfo = btRigidBody::btRigidBodyConstructionInfo;
	using Collider = btCollisionShape;
	using MotionState = EntityMotionState;

	RigidBody* body = nullptr;
	Collider* collider = nullptr;
	MotionState* motionState = nullptr;

	// TODO: Make setter and trigger Refilter
	u32 collisionGroups = 1<<0;
	bool trigger = false;

	ColliderComponent(bool _dynamic = false) : Component{this}, dynamic{_dynamic} {}
	void OnInit() override;
	void OnDestroy() override;

	void DisableRotation();
	void SetTrigger(bool);
	void SetAutosleep(bool);

	void Wakeup();
	void Refilter();

	vec3 GetVelocity() const;
	void SetVelocity(const vec3&);

	// TODO: Functions for getting and setting body properties
	//	velocity, damping, etc...

protected:
	bool dynamic = false;

	virtual void CreateCollider() = 0;
};

struct BoxColliderComponent : ColliderComponent {
	BoxColliderComponent(const vec3& _size = vec3{1.f}, bool _dynamic = false) : ColliderComponent{_dynamic}, size{_size} {}
	void CreateCollider() override;

	vec3 size;
};

struct SphereColliderComponent : ColliderComponent {
	SphereColliderComponent(f32 r = 1.f, bool _dynamic = false) : ColliderComponent{_dynamic}, radius{r} {}
	void CreateCollider() override;

	f32 radius;
};

struct CapsuleColliderComponent : ColliderComponent {
	CapsuleColliderComponent(f32 _radius = 1.f, f32 _height = 2.f, bool _dynamic = false) 
		: ColliderComponent{_dynamic}, radius{_radius}, height{_height} {}
	void CreateCollider() override;

	f32 radius, height;
};

// This is for level meshes and stuff that doesn't move ever.
struct MeshColliderComponent : ColliderComponent {
	MeshColliderComponent(bool _dynamic = false) : ColliderComponent{_dynamic} {}
	void CreateCollider() override;
};

// TODO: Mesh convex hull
// TODO: Other shapes


#endif