#include "physicsmanager.h"
#include "apptime.h"
#include "entity.h"
#include "app.h"

void OgreMatToF32Array(const mat4& m, f32 o[16]){
	for(u8 i = 0; i < 4; i++){
		o[4*0+i] = m[i][0];
		o[4*1+i] = m[i][1];
		o[4*2+i] = m[i][2];
		o[4*3+i] = m[i][3];
	}
}
mat4 F32ArrayToOgreMat(const f32* arr){
	mat4 o;
	for(u8 i = 0; i < 4; i++){
		o[0][i] = arr[i*4+0];
		o[1][i] = arr[i*4+1];
		o[2][i] = arr[i*4+2];
		o[3][i] = arr[i*4+3];
	}

	return o;
}

template<>
PhysicsManager* Singleton<PhysicsManager>::instance = nullptr;

PhysicsManager::PhysicsManager(f32 refreshRate)
	: timestep{1.f/refreshRate}, accumulator{0.f} {

	world = NewtonCreate();

	auto worldSizeub = vec3(1000.f, 100.f, 1000.f);
	auto worldSizelb = -worldSizeub;
	NewtonSetWorldSize(world, &worldSizelb.x, &worldSizeub.x);
	NewtonSetSolverModel(world, 1);

	enabledCollisionGroups = 0xff;
}

PhysicsManager::~PhysicsManager(){
	if(!world) return;
	NewtonDestroyAllBodies(world);
	NewtonDestroy(world);
}

void PhysicsManager::Update(){
	auto dt = std::min(AppTime::deltaTime, 1./4.);
	accumulator += dt;

	while(accumulator >= timestep){
		NewtonUpdate(world, timestep);
		accumulator -= timestep;
	}
}

extern "C" void SetTransformCallback(const NewtonBody* body, const f32* nmat, int){
	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));
	auto ent = comp->entity;

	auto mat = F32ArrayToOgreMat(nmat);

	vec3 position, scale;
	quat ori;

	mat.decomposition(position, scale, ori);
	ent->SetGlobalPosition(position);
	ent->SetScale(scale);
	ent->SetGlobalOrientation(ori);
}

extern "C" void ApplyGravityCallback(const NewtonBody* body, f32 dt, int){
	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));

	if(comp->IsType<ColliderComponent>()){
		auto col = static_cast<ColliderComponent*>(comp);
		NewtonBodySetForce(body, &col->force.x);
		col->force = vec3::ZERO;
	}
}

void ColliderComponent::OnInit() {
	auto world = PhysicsManager::GetSingleton()->world;
	mat4 omat = entity->GetFullTransform();
	omat = omat.transpose();

	CreateCollider();
	body = NewtonCreateBody(world, collider, omat[0]);

	if(dynamic) SetMassMatrix();

	NewtonBodySetUserData(body, this);
	NewtonBodySetTransformCallback(body, SetTransformCallback);
	NewtonBodySetForceAndTorqueCallback(body, ApplyGravityCallback);
}

void ColliderComponent::OnDestroy() {
	// TODO: Destroy newton body/collider
}

void ColliderComponent::ConstrainUpright(){
	NewtonConstraintCreateUpVector(PhysicsManager::GetSingleton()->world, &vec3::UNIT_Y.x, body);
}

void BoxColliderComponent::CreateCollider() {
	auto world = PhysicsManager::GetSingleton()->world;
	collider = NewtonCreateBox(world, 1, 1, 1, 0, nullptr);
}

void BoxColliderComponent::SetMassMatrix() {
	// TODO: Make members
	f32 mass = 1.;
	f32 lx = 1.;
	f32 ly = 1.;
	f32 lz = 1.;

	// http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetMassMatrix
	f32 Ixx = mass * (ly*ly + lz*lz) / 12;
	f32 Iyy = mass * (lx*lx + lz*lz) / 12;
	f32 Izz = mass * (lx*lx + ly*ly) / 12;

	NewtonBodySetMassMatrix(body, mass, Ixx, Iyy, Izz);
}


void CapsuleColliderComponent::CreateCollider() {
	auto world = PhysicsManager::GetSingleton()->world;
	quat rotation;
	mat4 transform;

	// This is required because for some reason Newton capsules
	//	lie along the x-axis
	rotation.FromAngleAxis(Ogre::Radian(M_PI/2.0), vec3::UNIT_Z);
	transform.makeTransform(vec3::ZERO, vec3(1,1,1), rotation);
	transform = transform.transpose();

	// TODO: Make members
	f32 r = 1.f, h = 2.f;

	collider = NewtonCreateCapsule(world, r, h, 0, transform[0]);
}

void CapsuleColliderComponent::SetMassMatrix() {
	// TODO: Make members
	f32 mass = 1.;
	f32 lx = 1.;
	f32 ly = 1.;
	f32 lz = 1.;

	// http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetMassMatrix
	f32 Ixx = mass * (ly*ly + lz*lz) / 12;
	f32 Iyy = mass * (lx*lx + lz*lz) / 12;
	f32 Izz = mass * (lx*lx + ly*ly) / 12;

	NewtonBodySetMassMatrix(body, mass, Ixx, Iyy, Izz);
}


// TODO: this
void StaticMeshColliderComponent::CreateCollider() {
	// auto world = PhysicsManager::GetSingleton()->world;
}

void StaticMeshColliderComponent::SetMassMatrix() {}