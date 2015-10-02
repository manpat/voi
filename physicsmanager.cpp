#include "entitymotionstate.h"
#include "physicsmanager.h"
#include "meshinfo.h"
#include "apptime.h"
#include "entity.h"
#include "app.h"

#include <OGRE/OgreEntity.h>
#include <algorithm>

template<>
PhysicsManager* Singleton<PhysicsManager>::instance = nullptr;

btVector3 o2bt(const vec3& v){
	return {v.x, v.y, v.z};
}
vec3 bt2o(const btVector3& v){
	return {v.x(), v.y(), v.z()};
}

void LayerNearCollisionFilterCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo) {
	auto proxy0 = collisionPair.m_pProxy0;
	auto proxy1 = collisionPair.m_pProxy1;

	auto ud0 = static_cast<btCollisionObject*>(proxy0->m_clientObject)->getUserPointer();
	auto ud1 = static_cast<btCollisionObject*>(proxy1->m_clientObject)->getUserPointer();

	auto comp0 = static_cast<Component*>(ud0);
	auto comp1 = static_cast<Component*>(ud1);

	auto col0 = comp0->As<ColliderComponent>(false);
	auto col1 = comp1->As<ColliderComponent>(false);

	if(!col0 || !col1) throw "One of the components in LayerNearCollisionFilterCallback isn't a collider";

	auto ecg = PhysicsManager::GetSingleton()->enabledCollisionGroups;
	auto mask = (col0->collisionGroups & col1->collisionGroups & ecg);

	if(mask == 0) return;

	// Continue physics stuff as usual
	dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
}

PhysicsManager::PhysicsManager(f32 refreshRate)
	: timestep{1.f/refreshRate} {

	auto collisionConfig = new btDefaultCollisionConfiguration{};
	broadphase = new Broadphase{};
	dispatcher = new Dispatcher{collisionConfig};
	solver = new Solver{};

	world = new World{dispatcher, broadphase, solver, collisionConfig};
	world->setGravity({0, -20., 0});
	dispatcher->setNearCallback(LayerNearCollisionFilterCallback);
}

PhysicsManager::~PhysicsManager(){
	if(!world) return;
	delete world;
	delete solver;
	delete dispatcher->getCollisionConfiguration();
	delete dispatcher;
	delete broadphase;
}

void PhysicsManager::Update(){
	world->stepSimulation((btScalar)AppTime::deltaTime, 10);
	currentStamp++;
	
	// Find all collisions between colliders and collider triggers
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)contactManifold->getBody0();
		btCollisionObject* obB = (btCollisionObject*)contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f) {
				auto ud0 = static_cast<btCollisionObject*>(obA)->getUserPointer();
				auto ud1 = static_cast<btCollisionObject*>(obB)->getUserPointer();

				auto col0 = static_cast<Component*>(ud0)->As<ColliderComponent>(false);
				auto col1 = static_cast<Component*>(ud1)->As<ColliderComponent>(false);

				// If both objects are valid colliders and either of them are triggers
				if(!col0 || !col1) continue;
				if(!col0->trigger && !col1->trigger) continue;

				// Process collision
				ProcessTriggerCollision(col0, col1);
			}
		}
	}

	// Remove stale trigger collisions and notify relevant
	//	entities
	for(auto& tcp: activeTriggerPairs){
		// Should work fine given unsigned integer underflow
		if((currentStamp - tcp->stamp) > 1){
			tcp->trigger->entity->SendMessage("leave", tcp->collider->entity);
			tcp->collider->entity->SendMessage("leave", tcp->trigger->entity);
			delete tcp;
			tcp = nullptr;
		}
	}

	// Remove all deleted and null trigger collisions
	activeTriggerPairs.erase(
		std::remove(activeTriggerPairs.begin(), activeTriggerPairs.end(), nullptr), 
		activeTriggerPairs.end());
}

void PhysicsManager::ProcessTriggerCollision(ColliderComponent* col0, ColliderComponent* col1){
	ColliderComponent* trigger = nullptr;
	ColliderComponent* col = nullptr;

	// Figure out which is the trigger and which is the collider
	if(col0->trigger){
		trigger = col0;
		col = col1;
	}

	if(col1->trigger){
		// Don't process two triggers
		if(trigger) return;
		trigger = col1;
		col = col0;
	}

	// Give up if neither are triggers
	if(!trigger) return;

	// Test if this collision is already being tracked
	auto it = std::find_if(activeTriggerPairs.begin(), activeTriggerPairs.end(), 
		[trigger, col](const TriggerColliderPair* tcp){
			return tcp->trigger == trigger && tcp->collider == col;
		});

	// If it already exists, update stamp and stop
	if(it != activeTriggerPairs.end()) {
		(*it)->stamp = currentStamp;
		return;
	}

	// Otherwise register new pair
	auto atp = new TriggerColliderPair{trigger, col, currentStamp};
	activeTriggerPairs.push_back(atp);

	// Notify relevant entities
	trigger->entity->SendMessage("enter", col->entity);
	col->entity->SendMessage("enter", trigger->entity);
}

void ColliderComponent::OnInit() {
	auto world = PhysicsManager::GetSingleton()->world;

	CreateCollider();
	motionState = new EntityMotionState(entity);

	btScalar mass = 0.;
	btVector3 inertia {0,0,0};
	if(dynamic) {
		mass = 10.;
		collider->calculateLocalInertia(mass, inertia);
	}

	RigidBodyInfo bodyInfo{mass, motionState, collider, inertia};
	// bodyInfo.m_friction = 1.0;
	// bodyInfo.m_rollingFriction = 0.3;

	body = new RigidBody{bodyInfo};
	body->setUserPointer(this);

	world->addRigidBody(body);
}

void ColliderComponent::OnDestroy() {
	// TODO: Destroy bullet body/collider
}

void ColliderComponent::DisableRotation(){
	body->setAngularFactor(0.f);
}

void ColliderComponent::SetTrigger(bool _trigger){
	// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Callbacks_and_Triggers
	// TODO: lookup ghost objects

	trigger = _trigger;

	auto flags = body->getCollisionFlags();
	if(trigger){
		flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
	}else{
		flags &= ~btCollisionObject::CF_NO_CONTACT_RESPONSE;
	}

	body->setCollisionFlags(flags);
}

void ColliderComponent::SetAutosleep(bool as){
	body->setActivationState(as?WANTS_DEACTIVATION:DISABLE_DEACTIVATION);
}

void ColliderComponent::Wakeup(){
	body->activate();
}

void ColliderComponent::Refilter(){
	// TODO: this

	// struct Dummy : PhysicsManager::World::ContactResultCallback {
	// 	RigidBody* body;
	// 	Dummy(RigidBody* b) : body{b} {}

	// 	btScalar addSingleResult(btManifoldPoint&, const btCollisionObjectWrapper*, int, int, const btCollisionObjectWrapper*, int, int){
	// 		return 0.;
	// 	}
	// 	bool needsCollision (btBroadphaseProxy *proxy) const{
	// 		if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
	// 			return false;

	// 		return body->checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
	// 	}
	// } dummyCB{body};

	// PhysicsManager::GetSingleton()->world->contactTest(body, dummyCB);
}

vec3 ColliderComponent::GetVelocity() const {
	return bt2o(body->getLinearVelocity());
}
void ColliderComponent::SetVelocity(const vec3& v){
	body->setLinearVelocity(o2bt(v));
}

void BoxColliderComponent::CreateCollider() {
	auto hs = size/2.f;
	collider = new btBoxShape{o2bt(hs)};
}

void CapsuleColliderComponent::CreateCollider() {
	collider = new btCapsuleShape{radius, height};
}

void SphereColliderComponent::CreateCollider() {
	collider = new btSphereShape{radius};
}

void MeshColliderComponent::CreateCollider() {
	auto trimesh = new btTriangleMesh();

	// Yuck
	for(u32 i = 0; i < entity->ogreEntity->getMesh()->getNumSubMeshes(); i++){
		auto sm = entity->ogreEntity->getMesh()->getSubMesh(i);
		auto smVertices = GetOgreSubMeshVertices(sm);

		for(u32 i = 0; i < smVertices.size()/3; i++) {
			btVector3 vs[] = {
				o2bt(smVertices[i*3+0]),
				o2bt(smVertices[i*3+1]),
				o2bt(smVertices[i*3+2]),
			};

			trimesh->addTriangle(vs[0], vs[1], vs[2]);
		}
	}

	collider = new btBvhTriangleMeshShape(trimesh, !dynamic /* Optimise for static */);
}
