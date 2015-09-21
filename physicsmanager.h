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
	bool dynamic = false;

	vec3 force = vec3::ZERO;

	ColliderComponent(bool _dynamic = false) : Component{this}, dynamic{_dynamic} {}
	void OnInit() override;
	void OnDestroy() override;

	void ConstrainUpright();

protected:
	virtual void CreateCollider() = 0;
	virtual void SetMassMatrix() = 0;
};

struct BoxColliderComponent : ColliderComponent {
	BoxColliderComponent(bool _dynamic = false) : ColliderComponent{_dynamic} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};

struct CapsuleColliderComponent : ColliderComponent {
	CapsuleColliderComponent(bool _dynamic = false) : ColliderComponent{_dynamic} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};

// This is for level meshes and stuff that doesn't move ever.
struct StaticMeshColliderComponent : ColliderComponent {
	StaticMeshColliderComponent() : ColliderComponent{false} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};


#endif