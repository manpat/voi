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


	ColliderComponent(bool dynamic = false) : Component{this}, dynamic{dynamic} {}
	void OnAwake() override;
	void OnDestroy() override;

	virtual void CreateCollider() = 0;
	virtual void SetMassMatrix() = 0;
};

struct BoxColliderComponent : ColliderComponent {
	BoxColliderComponent(bool dynamic = false) : ColliderComponent{dynamic} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};

struct StaticMeshColliderComponent : ColliderComponent {
	StaticMeshColliderComponent() : ColliderComponent{dynamic} {}
	void CreateCollider() override;
	void SetMassMatrix() override;
};

#endif