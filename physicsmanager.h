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

struct BoxColliderComponent : Component {
	NewtonBody* body = nullptr;
	NewtonCollision* collider = nullptr;
	vec3 force = vec3::ZERO;

	BoxColliderComponent() : Component{this} {}
	void OnInit() override;
	void OnDestroy() override;
};

#endif