#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include <Newton.h>
#include "common.h"
#include "singleton.h"
#include "component.h"

// http://newtondynamics.com/wiki/index.php5?title=API_Database

struct PhysicsManager : Singleton<PhysicsManager> {
	NewtonWorld* world;

	f32 timestep, accumulator;
	u32 enabledCollisionGroups;

	PhysicsManager(f32 = 60.f);
	~PhysicsManager();

	void Update();
};

struct ColliderComponent : Component {
	NewtonBody* body = nullptr;
	NewtonCollision* collider = nullptr;

	vec3 force = vec3::ZERO;
	vec3 velocity = vec3::ZERO;
	u32 collisionGroups = 1<<0;

	ColliderComponent(bool _dynamic = false) : Component{this}, dynamic{_dynamic} {}
	void OnInit() override;
	void OnDestroy() override;

	void ConstrainUpright();
	void SetTrigger(bool);

	// TODO: Functions for getting and setting body properties
	//	velocity, damping, etc...

protected:
	bool dynamic = false;

	virtual void CreateCollider() = 0;
	virtual void SetMassMatrix() = 0;
};

struct BoxColliderComponent : ColliderComponent {
	BoxColliderComponent(const vec3& _size = vec3{1.f}, bool _dynamic = false) : ColliderComponent{_dynamic}, size{_size} {}
	void CreateCollider() override;
	void SetMassMatrix() override;

	vec3 size;
};

struct SphereColliderComponent : ColliderComponent {
	SphereColliderComponent(f32 r = 1.f, bool _dynamic = false) : ColliderComponent{_dynamic}, radius{r} {}
	void CreateCollider() override;
	void SetMassMatrix() override;

	f32 radius;
};

struct CapsuleColliderComponent : ColliderComponent {
	CapsuleColliderComponent(f32 _radius = 1.f, f32 _height = 2.f, bool _dynamic = false) 
		: ColliderComponent{_dynamic}, radius{_radius}, height{_height} {}
	void CreateCollider() override;
	void SetMassMatrix() override;

	f32 radius, height;
};

// This is for level meshes and stuff that doesn't move ever.
struct StaticMeshColliderComponent : ColliderComponent {
	StaticMeshColliderComponent() : ColliderComponent{false} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};


#endif