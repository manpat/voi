#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include <vector>
#include <btBulletDynamicsCommon.h>
#include "common.h"
#include "singleton.h"
#include "component.h"

// Ray casting
// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Using_RayTest

// TODO: Non-mesh colliders are created with their center at origin
//	This is a problem because meshes don't have to be centered on their origin
//	
//	Probably have the blender plugin export an offset

struct ColliderComponent;

struct PhysicsManager : Singleton<PhysicsManager> {
	using Broadphase = btDbvtBroadphase;
	using Dispatcher = btCollisionDispatcher;
	using Solver = btSequentialImpulseConstraintSolver;
	using World = btDiscreteDynamicsWorld;

private: 
	struct ColliderPair {
		ColliderComponent* collider0;
		ColliderComponent* collider1;
		u8 stamp;
	};

public:
	struct RaycastResult {
		bool hit() const { return collider != nullptr; }
		operator bool() const { return hit(); }

		ColliderComponent* collider;
		vec3 hitPosition;
		vec3 hitNormal;
		f32 distance;
	};

	Broadphase* broadphase = nullptr;
	Dispatcher* dispatcher = nullptr;
	Solver* solver = nullptr;
	World* world = nullptr;

	// Stores pairs of triggers to colliding colliders and a timestamp
	//	to test for staleness. I'm not sure I like this implementation
	std::vector<ColliderPair*> activeColliderPairs;
	u8 currentStamp = 0;

	f32 timestep = 1.f/60.f;
	u32 enabledCollisionGroups = ~0u;

	PhysicsManager(f32 = 60.f);
	~PhysicsManager();

	void Update();

	// Linecast returns the first ColliderComponent hit between begin and end
	//	or nullptr if nothing. If layer is specified, rays will only hit
	//	colliders in that layer. Layer < 0 will hit all layers.
	RaycastResult Linecast(const vec3& begin, const vec3& end, s32 layer = -1, u32 collisionMask = ~0u);

	// Raycast is the same as Linecast except it takes direction*distance 
	//	instead of end
	RaycastResult Raycast(const vec3& begin, const vec3& dir, s32 layer = -1, u32 collisionMask = ~0u);

private:
	void ProcessCollision(ColliderComponent*, ColliderComponent*);
	void ProcessTriggerCollision(ColliderComponent*, ColliderComponent*);
};

class EntityMotionState;

// TODO: Colliders can/should be split into a separate module
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
	bool kinematic = false;

	vec3 dimensions = vec3::ZERO;

	ColliderComponent(const vec3& _dim, bool _dynamic = false) : Component{this}, 
		dimensions{_dim}, dynamic{_dynamic} {}
	void OnInit() override;
	void OnDestroy() override;

	void DisableRotation();
	void SetTrigger(bool);
	void SetKinematic(bool);
	void SetAutosleep(bool);

	void Wakeup();
	void Refilter();

	vec3 GetVelocity() const;
	void SetVelocity(const vec3&);

	vec3 GetPosition() const;
	void SetPosition(const vec3&);

protected:
	bool dynamic = false;

	virtual void CreateCollider() = 0;
};

struct BoxColliderComponent : ColliderComponent {
	BoxColliderComponent(const vec3& dm, bool dn = false) : ColliderComponent(dm, dn) {}
	void CreateCollider() override;
};

struct SphereColliderComponent : ColliderComponent {
	SphereColliderComponent(const vec3& dm, bool dn = false) : ColliderComponent(dm, dn) {}
	void CreateCollider() override;
};

struct CapsuleColliderComponent : ColliderComponent {
	CapsuleColliderComponent(const vec3& dm, bool dn = false) : ColliderComponent(dm, dn) {}
	void CreateCollider() override;
};

// This is mainly for level meshes
struct MeshColliderComponent : ColliderComponent {
	MeshColliderComponent(const vec3& dm, bool dn = false) : ColliderComponent(dm, dn) {}
	void CreateCollider() override;
};

// TODO: Mesh convex hull
// TODO: Other shapes


#endif