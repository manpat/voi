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
	return {o.x, o.y, o.z, o.w};
}
inline quat bt2o(const btQuaternion& o){
	return {o.x(), o.y(), o.z(), o.w()};
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

	if(ctx->needsRefilter) {
		// Clean the broadphase of AABB data
		btOverlappingPairCache* pairs = ctx->broadphase->getOverlappingPairCache();
		s32 numPairs = pairs->getNumOverlappingPairs();
		btBroadphasePairArray pairArray = pairs->getOverlappingPairArray();

		for(s32 i = 0; i < numPairs; i++) {
			btBroadphasePair& currPair = pairArray.at(i);
			pairs->removeOverlappingPair(currPair.m_pProxy0, currPair.m_pProxy1, ctx->dispatcher);
		}

		// Clean the dispatcher(narrowphase) of shape data
		s32 numManifolds = ctx->world->getDispatcher()->getNumManifolds();
		for(s32 i = 0; i < numManifolds; i++) {
			ctx->dispatcher->releaseManifold(ctx->dispatcher->getManifoldByIndexInternal(i));
		}

		ctx->broadphase->resetPool(ctx->dispatcher);
		ctx->solver->reset();
		ctx->needsRefilter = false;
	}

	ctx->world->stepSimulation((btScalar)dt, 10);
	ctx->currentStamp++;

	auto begin = ctx->activeColliderPairs.begin();
	auto end = ctx->activeColliderPairs.end();
	ctx->activeColliderPairs.erase(std::remove_if(begin, end, [](const PhysicsColliderPair& cp) {
		return !cp.entityID0 || !cp.entityID1;
	}), end);

	// Find all collisions between colliders and collider triggers
	auto dispatcher = ctx->world->getDispatcher();
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
		auto obA = (btCollisionObject*)contactManifold->getBody0();
		auto obB = (btCollisionObject*)contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f) {
				// NOTE: Relies on unsigned underflow
				auto entIdx0 = (u16) (size_t) obA->getUserPointer()-1;
				auto entIdx1 = (u16) (size_t) obB->getUserPointer()-1;

				if(entIdx0 >= scene->numEntities) continue;
				if(entIdx1 >= scene->numEntities) continue;

				auto ent0 = &scene->entities[entIdx0];
				auto ent1 = &scene->entities[entIdx1];

				if(ent0->entityType == Entity::TypeTrigger) {
					ProcessTriggerCollision(ctx, ent0, ent1);
				}else if(ent1->entityType == Entity::TypeTrigger) {
					ProcessTriggerCollision(ctx, ent1, ent0);
				}else{
					ProcessCollision(ctx, ent0, ent1);
				}
			}
		}
	}

	// Remove stale trigger collisions and notify relevant
	//	entities
	for(auto& cp: ctx->activeColliderPairs){
		if(!cp.entityID0 || !cp.entityID1) continue;

		// Should work fine given unsigned integer underflow
		if((ctx->currentStamp - cp.stamp) > 1u){
			auto ent0 = &scene->entities[cp.entityID0-1];
			auto ent1 = &scene->entities[cp.entityID1-1];
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
	}
}

void LayerNearCollisionFilterCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo) {
	auto proxy0 = collisionPair.m_pProxy0;
	auto proxy1 = collisionPair.m_pProxy1;

	auto ud0 = static_cast<btCollisionObject*>(proxy0->m_clientObject)->getUserPointer();
	auto ud1 = static_cast<btCollisionObject*>(proxy1->m_clientObject)->getUserPointer();

	auto ent0 = static_cast<Entity*>(ud0);
	auto ent1 = static_cast<Entity*>(ud1);

	auto mask = (ent0->layers & ent1->layers);
	if(mask == 0) return;

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

bool InitEntityPhysics(Scene* scene, Entity* ent, const MeshData* meshdata) {
	// bool scaled = glm::length(ent->scale-1.f) < 1e-9;

	// TODO: Proper sizes
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
		ent->collider = new btCapsuleShape{radius, ent->extents.y-2*radius};
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
		ent->collider = new btBvhTriangleMeshShape{btmesh, ent->flags & Entity::FlagStatic};
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

	}else if(ent->entityType == Entity::TypePlayer) {
		ent->rigidbody->setActivationState(DISABLE_DEACTIVATION);
		ent->rigidbody->setFriction(0.f);
	}

	ent->collider->setLocalScaling(o2bt(ent->scale));

	scene->physicsContext.world->addRigidBody(ent->rigidbody);

	return true;
}

void DeinitEntityPhysics(Scene* scene, Entity* ent) {
	auto ctx = &scene->physicsContext;
	
	for(auto& cp: ctx->activeColliderPairs){
		if(!cp.entityID0 || !cp.entityID1) continue;
		if(cp.entityID0 != ent->id && cp.entityID1 != ent->id) continue;

		if(cp.entityID0 > scene->numEntities || cp.entityID0 > scene->numEntities){
			printf("Warning! An active collider pair was found with an invalid entity ID!\n");
			cp.entityID0 = 0;
			cp.entityID1 = 0;
			continue;
		}

		auto ent0 = &scene->entities[cp.entityID0-1];
		auto ent1 = &scene->entities[cp.entityID1-1];

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

	ctx->world->removeRigidBody(ent->rigidbody);
	delete ent->rigidbody->getMotionState();
	delete ent->rigidbody;
	delete ent->collider;

	ent->rigidbody = nullptr;
	ent->collider = nullptr;
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