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
	world->stepSimulation(timestep, 10);
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
	// auto world = PhysicsManager::GetSingleton()->world;
	// mat4 omat = entity->GetFullTransform();
	// omat = omat.transpose();

	// CreateCollider();
	// body = NewtonCreateBody(world, collider, omat[0]);

	// if(dynamic) SetMassMatrix();

	// NewtonBodySetUserData(body, this);
	// NewtonBodySetTransformCallback(body, SetTransformCallback);
	// NewtonBodySetForceAndTorqueCallback(body, ApplyForceCallback);
}

void ColliderComponent::OnDestroy() {
	// TODO: Destroy newton body/collider
}

void ColliderComponent::ConstrainUpright(){
	// NewtonConstraintCreateUpVector(PhysicsManager::GetSingleton()->world, &vec3::UNIT_Y.x, body);
}

void ColliderComponent::SetTrigger(bool trigger){
	(void) trigger;
	// NewtonCollisionSetAsTriggerVolume(collider, trigger);
}

void BoxColliderComponent::CreateCollider() {
	// auto world = PhysicsManager::GetSingleton()->world;
	// collider = NewtonCreateBox(world, size.x, size.y, size.z, 0, nullptr);
}


void CapsuleColliderComponent::CreateCollider() {
	// auto world = PhysicsManager::GetSingleton()->world;
	// quat rotation;
	// mat4 transform;

	// // This is required because for some reason Newton capsules
	// //	lie along the x-axis
	// rotation.FromAngleAxis(Ogre::Radian(M_PI/2.0), vec3::UNIT_Z);
	// transform.makeTransform(vec3::ZERO, vec3(1,1,1), rotation);
	// transform = transform.transpose();

	// collider = NewtonCreateCapsule(world, radius, height, 0, transform[0]);
}


void SphereColliderComponent::CreateCollider() {
	// auto world = PhysicsManager::GetSingleton()->world;
	// collider = NewtonCreateSphere(world, radius, radius, radius, 0, nullptr);
}


void StaticMeshColliderComponent::CreateCollider() {
	// auto world = PhysicsManager::GetSingleton()->world;
	// collider = NewtonCreateTreeCollision(world, 0);

	// // Temporary
	// auto sm = entity->ogreEntity->getMesh()->getSubMesh(0);
	// auto smVertices = GetOgreSubMeshVertices(sm);

	// NewtonTreeCollisionBeginBuild(collider);
	// for(u32 i = 0; i < smVertices.size()/3; i++){
	// 	NewtonTreeCollisionAddFace(collider, 3, &smVertices[i*3].x, sizeof(vec3), 1);
	// }

	// NewtonTreeCollisionEndBuild(collider, 1);
}
