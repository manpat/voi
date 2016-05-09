#include "voi.h"
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

void UpdatePhysics(PhysicsContext* ctx, Scene* scene, f32 dt) {
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
				// TODO
				// cp.entityID0->entity->OnTriggerLeave(cp.entityID1);
				// cp.entityID1->entity->OnTriggerLeave(cp.entityID0);
			}else{
				// TODO
				// cp.entityID0->entity->OnCollisionLeave(cp.entityID1);
				// cp.entityID1->entity->OnCollisionLeave(cp.entityID0);
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

	(void)ud0;
	(void)ud1;

	// TODO
	// auto comp0 = static_cast<Component*>(ud0);
	// auto comp1 = static_cast<Component*>(ud1);

	// auto col0 = comp0->As<ColliderComponent>(false);
	// auto col1 = comp1->As<ColliderComponent>(false);

	// if(!col0 || !col1) {
	// 	std::cout << "One of the components in LayerNearCollisionFilterCallback isn't a collider" << std::endl;
	// 	return;
	// }

	// auto ecg = PhysicsManager::GetSingleton()->enabledCollisionGroups;
	// auto mask = (col0->collisionGroups & col1->collisionGroups & ecg);

	// if(mask == 0) return;

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

	// TODO
	// Notify relevant entities
	// col0->entity->OnCollisionEnter(col1);
	// col1->entity->OnCollisionEnter(col0);
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

	// TODO
	// Notify relevant entities
	// trigger->entity->OnTriggerEnter(col);
	// col->entity->OnTriggerEnter(trigger);
}