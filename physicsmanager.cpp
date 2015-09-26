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

PhysicsManager::PhysicsManager(f32 refreshRate)
	: timestep{1.f/refreshRate} {

	auto collisionConfig = new btDefaultCollisionConfiguration{};
	broadphase = new Broadphase{};
	dispatcher = new Dispatcher{collisionConfig};
	solver = new Solver{};

	world = new World{dispatcher, broadphase, solver, collisionConfig};
	world->setGravity({0, -10., 0});

	enabledCollisionGroups = 0xff;
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
	world->stepSimulation(AppTime::deltaTime, 10);
}

// extern "C" s32 FilterCollision(const NewtonMaterial* mat, const NewtonBody* b1, const NewtonBody* b2, s32){
// 	auto ud = NewtonMaterialGetMaterialPairUserData(mat);
// 	auto physMan = static_cast<PhysicsManager*>(ud);

// 	auto ud1 = NewtonBodyGetUserData(b1);
// 	auto ud2 = NewtonBodyGetUserData(b2);
// 	auto collider1 = static_cast<Component*>(ud1)->As<ColliderComponent>();
// 	auto collider2 = static_cast<Component*>(ud2)->As<ColliderComponent>();

// 	return (s32)((collider1->collisionGroups 
// 		& collider2->collisionGroups & physMan->enabledCollisionGroups) > 0);
// }

// extern "C" void CollisionCallback (const NewtonJoint* const contact, f32, int){
// 	auto ud1 = NewtonBodyGetUserData(NewtonJointGetBody0(contact));
// 	auto ud2 = NewtonBodyGetUserData(NewtonJointGetBody1(contact));
// 	auto collider1 = static_cast<Component*>(ud1)->As<ColliderComponent>();
// 	auto collider2 = static_cast<Component*>(ud2)->As<ColliderComponent>();

// 	std::cout << "Collide " << collider1->id << " -> " << collider2->id << std::endl;
// }

// extern "C" void BodyLeaveWorldCallback(const NewtonBody* const body, int) {
// 	mat4 trans = mat4::getTrans(vec3{0,0,0}).transpose();

// 	NewtonBodySetMatrix(body, trans[0]);
// }

// extern "C" void SetTransformCallback(const NewtonBody* body, const f32* nmat, int){
// 	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));
// 	auto ent = comp->entity;

// 	auto mat = F32ArrayToOgreMat(nmat);

// 	vec3 position, scale;
// 	quat ori;

// 	mat.decomposition(position, scale, ori);
// 	ent->SetGlobalPosition(position);
// 	ent->SetScale(scale);
// 	ent->SetGlobalOrientation(ori);

// 	// Update collider velocity
// 	if(comp->IsType<ColliderComponent>()){
// 		auto col = static_cast<ColliderComponent*>(comp);
// 		NewtonBodyGetVelocity(col->body, &col->velocity.x);
// 	}
// }

// extern "C" void ApplyForceCallback(const NewtonBody* body, f32 dt, int){
// 	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));

// 	if(comp->IsType<ColliderComponent>()){
// 		auto col = static_cast<ColliderComponent*>(comp);
// 		auto force = col->force + vec3{0, -30., 0}; // Gravity

// 		NewtonBodySetVelocity(body, &col->velocity.x);
// 		NewtonBodyAddForce(body, &force.x);
// 		col->force = vec3::ZERO;
// 		col->velocity = vec3::ZERO;
// 	}
// }

void ColliderComponent::OnInit() {
	auto world = PhysicsManager::GetSingleton()->world;

	CreateCollider();
	motionState = new EntityMotionState(entity);

	btScalar mass = 0.;
	btVector3 inertia {0,0,0};
	if(dynamic) {
		mass = 1.;
		collider->calculateLocalInertia(mass, inertia);
	}

	body = new RigidBody{mass, motionState, collider, inertia};

	world->addRigidBody(body);

	// NewtonBodySetUserData(body, this);
	// NewtonBodySetTransformCallback(body, SetTransformCallback);
	// NewtonBodySetForceAndTorqueCallback(body, ApplyForceCallback);
}

void ColliderComponent::OnDestroy() {
	// TODO: Destroy bullet body/collider
}

void ColliderComponent::DisableRotation(){
	body->setAngularFactor(0.f);
}

void ColliderComponent::SetTrigger(bool trigger){
	(void) trigger;
	// NewtonCollisionSetAsTriggerVolume(collider, trigger);
}

void ColliderComponent::SetAutosleep(bool as){
	body->setActivationState(as?WANTS_DEACTIVATION:DISABLE_DEACTIVATION);
}

void ColliderComponent::Wakeup(){
	body->activate();
}

vec3 ColliderComponent::GetVelocity() const {
	auto v = body->getLinearVelocity();
	return vec3{v.x(), v.y(), v.z()};
}
void ColliderComponent::SetVelocity(const vec3& v){
	body->setLinearVelocity({v.x, v.y, v.z});
}

void BoxColliderComponent::CreateCollider() {
	auto hs = size/2.f;
	collider = new btBoxShape{{hs.x, hs.y, hs.z}};
}

void CapsuleColliderComponent::CreateCollider() {
	collider = new btCapsuleShape{radius, height};
}

void SphereColliderComponent::CreateCollider() {
	collider = new btSphereShape{radius};
}

void StaticMeshColliderComponent::CreateCollider() {
	collider = new btEmptyShape{};

	// // Temporary
	// auto sm = entity->ogreEntity->getMesh()->getSubMesh(0);
	// auto smVertices = GetOgreSubMeshVertices(sm);

	// NewtonTreeCollisionBeginBuild(collider);
	// for(u32 i = 0; i < smVertices.size()/3; i++){
	// 	NewtonTreeCollisionAddFace(collider, 3, &smVertices[i*3].x, sizeof(vec3), 1);
	// }

	// NewtonTreeCollisionEndBuild(collider, 1);
}
