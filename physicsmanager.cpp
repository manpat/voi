#include "entitymotionstate.h"
#include "physicsmanager.h"
#include "meshinfo.h"
#include "apptime.h"
#include "entity.h"
#include "app.h"

#include <OGRE/OgreEntity.h>

template<>
PhysicsManager* Singleton<PhysicsManager>::instance = nullptr;

// extern "C" void BodyLeaveWorldCallback(const NewtonBody* const body, int threadIndex);
// extern "C" s32 FilterCollision(const NewtonMaterial* mat, const NewtonBody* b1, const NewtonBody* b2, s32);
// extern "C" void CollisionCallback (const NewtonJoint* const contact, f32 timestep, int);

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
	world->setGravity({0, -10., 0});
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

void ColliderComponent::SetTrigger(bool trigger){
	(void) trigger;
	// http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Callbacks_and_Triggers
	// NewtonCollisionSetAsTriggerVolume(collider, trigger);
}

void ColliderComponent::SetAutosleep(bool as){
	body->setActivationState(as?WANTS_DEACTIVATION:DISABLE_DEACTIVATION);
}

void ColliderComponent::Wakeup(){
	body->activate();
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

void StaticMeshColliderComponent::CreateCollider() {
	collider = new btEmptyShape{};

	// TODO: Use all submeshes
	auto sm = entity->ogreEntity->getMesh()->getSubMesh(0);
	auto smVertices = GetOgreSubMeshVertices(sm);

	auto trimesh = new btTriangleMesh();

	for(u32 i = 0; i < smVertices.size()/3; i++) {
		btVector3 vs[] = {
			o2bt(smVertices[i*3+0]),
			o2bt(smVertices[i*3+1]),
			o2bt(smVertices[i*3+2]),
		};

		trimesh->addTriangle(vs[0], vs[1], vs[2]);
	}

	collider = new btBvhTriangleMeshShape(trimesh, !dynamic /* Optimise for static */);
}
