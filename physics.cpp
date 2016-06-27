#include "voi.h"
#include "sceneloader.h" // For MeshData
#include <algorithm>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <btBulletDynamicsCommon.h>

inline btVector3 o2bt(const vec3& v){
	return {v.x, v.y, v.z};
}
inline vec3 bt2o(const btVector3& v){
	return {v.x(), v.y(), v.z()};
}

inline btQuaternion o2bt(const quat& o){
	return btQuaternion(o.x, o.y, o.z, o.w);
}
inline quat bt2o(const btQuaternion& o){
	return quat(o.x(), o.y(), o.z(), o.w());
}

void LayerNearCollisionFilterCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
void ProcessCollision(PhysicsContext*, Entity*, Entity*);
void ProcessTriggerCollision(PhysicsContext*, Entity*, Entity*);

bool InitPhysics(PhysicsContext* ctx) {
	auto collisionConfig = new btDefaultCollisionConfiguration{};
	ctx->broadphase = new btDbvtBroadphase{};
	ctx->dispatcher = new btCollisionDispatcher{collisionConfig};
	ctx->solver = new btSequentialImpulseConstraintSolver{};

	ctx->world = new btDiscreteDynamicsWorld{ctx->dispatcher, ctx->broadphase, ctx->solver, collisionConfig};
	ctx->world->setGravity({0, -30., 0});
	ctx->dispatcher->setNearCallback(LayerNearCollisionFilterCallback);

	return true;
}

void UpdatePhysics(Scene* scene, f32 dt) {
	auto ctx = &scene->physicsContext;

	ctx->world->stepSimulation((btScalar)dt, 10);
	ctx->currentStamp++;

	auto begin = ctx->activeColliderPairs.begin();
	auto end = ctx->activeColliderPairs.end();
	ctx->activeColliderPairs.erase(std::remove_if(begin, end, [](const PhysicsColliderPair& cp) {
		return !cp.entityID0 || !cp.entityID1;
	}), end);

	// Find all collisions between colliders and collider triggers
	s32 numManifolds = ctx->dispatcher->getNumManifolds();
	for (s32 i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = ctx->dispatcher->getManifoldByIndexInternal(i);
		auto obA = (btCollisionObject*)contactManifold->getBody0();
		auto obB = (btCollisionObject*)contactManifold->getBody1();
		if(!obA || !obB) {
			puts("Warning! Contact manifold found with invalid rigidbody!");
			continue;
		}

		auto ent0 = (Entity*) obA->getUserPointer();
		auto ent1 = (Entity*) obB->getUserPointer();
		if(!ent0 || !ent1) {
			puts("Warning! Rigidbody found without user pointer set!");
			continue;
		}
		
		s32 numContacts = contactManifold->getNumContacts();
		for (s32 j = 0; j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if(pt.getDistance() < 0.5f) {
				if(ent0->entityType == Entity::TypeTrigger) {
					ProcessTriggerCollision(ctx, ent0, ent1);
				}else if(ent1->entityType == Entity::TypeTrigger) {
					ProcessTriggerCollision(ctx, ent1, ent0);
				}else{
					ProcessCollision(ctx, ent0, ent1);
				}
				break;
			}
		}
	}

	// Remove stale trigger collisions and notify relevant
	//	entities
	for(auto& cp: ctx->activeColliderPairs){
		if(!cp.entityID0 || !cp.entityID1) continue;

		if((u32)(ctx->currentStamp - cp.stamp) > 1u){
			auto ent0 = GetEntity(cp.entityID0);
			auto ent1 = GetEntity(cp.entityID1);

			if(ent0 && ent1) {
				if(ent0->entityType == Entity::TypeTrigger
				|| ent1->entityType == Entity::TypeTrigger){
					EntityOnTriggerLeave(ent0, ent1);
					EntityOnTriggerLeave(ent1, ent0);
				}else{
					EntityOnCollisionLeave(ent0, ent1);
					EntityOnCollisionLeave(ent1, ent0);
				}
			}

			// Setting these to 0 flags them for cleanup
			cp.entityID0 = 0;
			cp.entityID1 = 0;
		}
	}
}

void DeinitPhysics(PhysicsContext* ctx) {
	delete ctx->world;
	delete ctx->solver;
	delete ctx->dispatcher->getCollisionConfiguration();
	delete ctx->dispatcher;
	delete ctx->broadphase;

	ctx->world = nullptr;
	ctx->solver = nullptr;
	ctx->dispatcher = nullptr;
	ctx->broadphase = nullptr;
}

void RefilterEntity(Entity* e) {
	if(!e->rigidbody) return;
	if(!e->scene) {
		printf("Warning! Tried to refilter entity \"%.*s\" that doesn't belong to a scene\n",
			(u32)e->nameLength, e->name);
		return;
	}

	auto ctx = &e->scene->physicsContext;
	ctx->world->removeRigidBody(e->rigidbody);
	ctx->world->addRigidBody(e->rigidbody);
}

struct RaycastCallback : btCollisionWorld::ClosestRayResultCallback {
	RaycastCallback(const btVector3& start, const btVector3& end, u32 _layerMask = ~0u)
		: ClosestRayResultCallback{start, end}, layerMask{_layerMask} {}

	u32 layerMask;

	bool needsCollision(btBroadphaseProxy* proxy0) const override {
		auto body = (btRigidBody*)proxy0->m_clientObject;
		auto ent = (Entity*)(body?body->getUserPointer():nullptr);

		return ent && (ent->layers & layerMask);
	}
};

RaycastResult Raycast(Scene* scn, const vec3& o, const vec3& d, f32 maxDist, u32 layermask) {
	return Linecast(scn, o, o + d*maxDist, layermask);
}

RaycastResult Linecast(Scene* scn, const vec3& s, const vec3& e, u32 layermask) {
	auto start = o2bt(s);
	auto end = o2bt(e);

	RaycastCallback rayCallback{start, end, layermask};

	// Perform raycast
	scn->physicsContext.world->rayTest(start, end, rayCallback);

	if(rayCallback.hasHit()) {
		end = rayCallback.m_hitPointWorld;
		auto normal = rayCallback.m_hitNormalWorld;
		auto col = (Entity*)rayCallback.m_collisionObject->getUserPointer();
		f32 frac = rayCallback.m_closestHitFraction;
		f32 dist = glm::length(e-s);

		return {col, bt2o(end), bt2o(normal), frac*dist};
	}

	return {nullptr, vec3{0.f}, vec3{0.f}, std::numeric_limits<f32>::infinity()};
}


void LayerNearCollisionFilterCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo) {
	auto proxy0 = collisionPair.m_pProxy0;
	auto proxy1 = collisionPair.m_pProxy1;

	auto ud0 = static_cast<btCollisionObject*>(proxy0->m_clientObject)->getUserPointer();
	auto ud1 = static_cast<btCollisionObject*>(proxy1->m_clientObject)->getUserPointer();

	auto ent0 = static_cast<Entity*>(ud0);
	auto ent1 = static_cast<Entity*>(ud1);

	auto mask = (ent0->layers & ent1->layers);
	if(!mask) return;

	// Continue physics stuff as usual
	dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
}


void ProcessCollision(PhysicsContext* ctx, Entity* ent0, Entity* ent1){
	// Test if this collision is already being tracked
	auto begin = ctx->activeColliderPairs.begin();
	auto end = ctx->activeColliderPairs.end();
	auto it = std::find_if(begin, end,
		[ent0, ent1](const PhysicsColliderPair& cp){
			return (cp.entityID0 == ent0->id && cp.entityID1 == ent1->id)
				|| (cp.entityID0 == ent1->id && cp.entityID1 == ent0->id);
		});

	// If it already exists, update stamp and stop
	if(it != end) {
		it->stamp = ctx->currentStamp;
		return;
	}

	// Otherwise register new pair
	ctx->activeColliderPairs.push_back({ent0->id, ent1->id, ctx->currentStamp});

	// Notify relevant entities
	EntityOnCollisionEnter(ent0, ent1);
	EntityOnCollisionEnter(ent1, ent0);
}

void ProcessTriggerCollision(PhysicsContext* ctx, Entity* trigger, Entity* entity){
	// Test if this collision is already being tracked
	auto begin = ctx->activeColliderPairs.begin();
	auto end = ctx->activeColliderPairs.end();
	auto it = std::find_if(begin, end,
		[trigger, entity](const PhysicsColliderPair& cp){
			return (cp.entityID0 == trigger->id && cp.entityID1 == entity->id)
				|| (cp.entityID0 == entity->id && cp.entityID1 == trigger->id);
		});

	// If it already exists, update stamp and stop
	if(it != end) {
		it->stamp = ctx->currentStamp;
		return;
	}

	// Otherwise register new pair
	ctx->activeColliderPairs.push_back({trigger->id, entity->id, ctx->currentStamp});

	// Notify relevant entities
	EntityOnTriggerEnter(trigger, entity);
	EntityOnTriggerEnter(entity, trigger);
}

struct EntityMotionState : public btMotionState {
	Entity* entity;

	EntityMotionState(Entity* e) : entity{e} {}

	void getWorldTransform(btTransform& worldTrans) const override {
		// This gets called ONCE for non-kinematic bodies
		// It gets called every frame for kinematic bodies
		worldTrans.setIdentity();
		worldTrans.setOrigin(o2bt(entity->position));
		worldTrans.setRotation(o2bt(entity->rotation));
	}

	void setWorldTransform(const btTransform& newTrans) override {
		auto ori = newTrans.getRotation();
		auto pos = newTrans.getOrigin();
		entity->position = bt2o(pos);
		entity->rotation = bt2o(ori);
	}
};

bool InitEntityPhysics(Entity* ent, const MeshData* meshdata) {
	auto scene = ent->scene;
	// bool scaled = glm::length(ent->scale-1.f) < 1e-9;

	switch(ent->colliderType) {
	case ColliderCube:
		ent->collider = new btBoxShape{o2bt(ent->extents)};
		break;
	case ColliderCylinder:
		ent->collider = new btCylinderShape{o2bt(ent->extents)};
		break;
	case ColliderCapsule:{
		// NOTE: Caps aren't included in height. Total height = height + 2*radius
		auto radius = glm::max(ent->extents.x, ent->extents.z);
		ent->collider = new btCapsuleShape{radius, (ent->extents.y-radius)*2.f};
	}	break;

	case ColliderConvex: {
		if(!ent->meshID || !meshdata) {
			printf("Error! Entity '%.*s' has Mesh collider type but no mesh!\n",
				(u32)ent->nameLength, ent->name);
			return false;
		}

		if(ent->meshID > scene->numMeshes) {
			printf("Error! Entity '%.*s' has Mesh collider type but has an invalid mesh ID!\n",
				(u32)ent->nameLength, ent->name);
			return false;
		}

		auto hull = new btConvexHullShape{};
		auto verts = meshdata->vertices;

		for(u32 i = 0; i < meshdata->numVertices; i++) {
			hull->addPoint(o2bt(verts[i]));
		}

		ent->collider = hull;
	}	break;

	case ColliderMesh:{
		if(!ent->meshID || !meshdata) {
			printf("Error! Entity '%.*s' has Mesh collider type but no mesh!\n",
				(u32)ent->nameLength, ent->name);
			return false;
		}

		if(ent->meshID > scene->numMeshes) {
			printf("Error! Entity '%.*s' has Mesh collider type but has an invalid mesh ID!\n",
				(u32)ent->nameLength, ent->name);
			return false;
		}

		auto btmesh = new btTriangleMesh{};
		auto mesh = &scene->meshes[ent->meshID-1];
		auto verts = meshdata->vertices;

		for(u32 i = 0; i < mesh->numTriangles; i++) {
			u32 idx[3] = {0};

			switch(mesh->elementType) {
			case 0:
				idx[0] = meshdata->triangles8[i*3+0];
				idx[1] = meshdata->triangles8[i*3+1];
				idx[2] = meshdata->triangles8[i*3+2];
				break;
			case 1:
				idx[0] = meshdata->triangles16[i*3+0];
				idx[1] = meshdata->triangles16[i*3+1];
				idx[2] = meshdata->triangles16[i*3+2];
				break;
			case 2:
				idx[0] = meshdata->triangles32[i*3+0];
				idx[1] = meshdata->triangles32[i*3+1];
				idx[2] = meshdata->triangles32[i*3+2];
				break;
			}

			btmesh->addTriangle(
				o2bt(verts[idx[0]]), 
				o2bt(verts[idx[1]]), 
				o2bt(verts[idx[2]])
			);
		}

		// TODO: Look into btGimpactTriangleMeshShape for moving mesh colliders
		ent->collider = new btBvhTriangleMeshShape{btmesh, bool(ent->flags & Entity::FlagStatic)};
	}	break;

	case ColliderNone:
		ent->collider = nullptr;
		return true;
	default:
		printf("Error! Entity '%.*s' has unknown collider type!",
			(u32)ent->nameLength, ent->name);
		ent->collider = nullptr;
		return false;
	}

	btScalar mass = 0.f;
	btVector3 inertia {0,0,0};
	if(~ent->flags & Entity::FlagStatic) {
		mass = 100.; // NOTE: Super arbitrary
		ent->collider->calculateLocalInertia(mass, inertia);
	}

	auto motionState = new EntityMotionState{ent};

	btRigidBody::btRigidBodyConstructionInfo bodyInfo{
		mass, motionState, ent->collider, inertia
	};

	bodyInfo.m_rollingFriction = 0.f;

	ent->rigidbody = new btRigidBody{bodyInfo};
	ent->rigidbody->setUserPointer(ent);

	if(ent->entityType == Entity::TypeTrigger) {
		auto flags = ent->rigidbody->getCollisionFlags();
		flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
		ent->rigidbody->setCollisionFlags(flags);

	}else if(ent->entityType == Entity::TypePortal) {
		auto flags = ent->rigidbody->getCollisionFlags();
		flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
		ent->rigidbody->setCollisionFlags(flags);

	}else if(ent->entityType == Entity::TypePlayer) {
		ent->rigidbody->setActivationState(DISABLE_DEACTIVATION);
		ent->rigidbody->setFriction(0.f);
	}

	ent->collider->setLocalScaling(o2bt(ent->scale));

	scene->physicsContext.world->addRigidBody(ent->rigidbody);

	return true;
}

void DeinitEntityPhysics(Entity* ent) {
	auto scene = ent->scene;
	auto ctx = &scene->physicsContext;
	
	for(auto& cp: ctx->activeColliderPairs){
		if(!cp.entityID0 || !cp.entityID1) continue;
		if(cp.entityID0 != ent->id && cp.entityID1 != ent->id) continue;

		auto ent0 = GetEntity(cp.entityID0);
		auto ent1 = GetEntity(cp.entityID1);

		if(!ent0 || !ent1){
			printf("Warning! An active collider pair was found with an invalid entity ID!\n");
			cp.entityID0 = 0;
			cp.entityID1 = 0;
			continue;
		}

		if(ent0->entityType == Entity::TypeTrigger
		|| ent1->entityType == Entity::TypeTrigger){
			EntityOnTriggerLeave(ent0, ent1);
			EntityOnTriggerLeave(ent1, ent0);
		}else{
			EntityOnCollisionLeave(ent0, ent1);
			EntityOnCollisionLeave(ent1, ent0);
		}

		// Setting these to 0 flags them for cleanup
		cp.entityID0 = 0;
		cp.entityID1 = 0;
	}

	if(ent->rigidbody) {
		ctx->world->removeRigidBody(ent->rigidbody);
		delete ent->rigidbody->getMotionState();
		delete ent->rigidbody;
	}
	delete ent->collider;

	ent->rigidbody = nullptr;
	ent->collider = nullptr;
}

void SetEntityRotation(Entity* e, const quat& q) {
	auto trans = e->rigidbody->getCenterOfMassTransform();
	trans.setRotation(o2bt(q));
	e->rigidbody->setCenterOfMassTransform(trans);
}

void SetEntityVelocity(Entity* e, const vec3& v) {
	e->rigidbody->setLinearVelocity(o2bt(v));
}

vec3 GetEntityVelocity(const Entity* e) {
	return bt2o(e->rigidbody->getLinearVelocity());
}

vec3 GetEntityCenterOfMass(const Entity* e) {
	return bt2o(e->rigidbody->getCenterOfMassPosition());
}

void ConstrainEntityUpright(Entity* e) {
	e->rigidbody->setAngularFactor(0.f);
}

void SetEntityKinematic(Entity* e, bool k, bool setActivationState) {
	auto flags = e->rigidbody->getCollisionFlags();
	if(k){
		flags |= btCollisionObject::CF_KINEMATIC_OBJECT;
	}else{
		flags &= ~btCollisionObject::CF_KINEMATIC_OBJECT;
	}
	e->rigidbody->setCollisionFlags(flags);

	if(setActivationState){
		e->rigidbody->setActivationState(k?DISABLE_DEACTIVATION:ACTIVE_TAG);
	}

	RefilterEntity(e);
}
