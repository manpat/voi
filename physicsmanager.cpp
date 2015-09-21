#include "physicsmanager.h"
#include "apptime.h"
#include "entity.h"
#include "app.h"

// This transposes the matrix as well
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

	// Update collider velocity
	if(comp->IsType<ColliderComponent>()){
		auto col = static_cast<ColliderComponent*>(comp);
		NewtonBodyGetVelocity(col->body, &col->velocity.x);
	}
}

extern "C" void ApplyForceCallback(const NewtonBody* body, f32 dt, int){
	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));

	if(comp->IsType<ColliderComponent>()){
		auto col = static_cast<ColliderComponent*>(comp);
		auto force = col->force + vec3{0, -30., 0}; // Gravity

		NewtonBodySetVelocity(body, &col->velocity.x);
		NewtonBodyAddForce(body, &force.x);
		col->force = vec3::ZERO;
		col->velocity = vec3::ZERO;
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
	NewtonBodySetForceAndTorqueCallback(body, ApplyForceCallback);
}

void ColliderComponent::OnDestroy() {
	// TODO: Destroy newton body/collider
}

void ColliderComponent::ConstrainUpright(){
	NewtonConstraintCreateUpVector(PhysicsManager::GetSingleton()->world, &vec3::UNIT_Y.x, body);
}

void BoxColliderComponent::CreateCollider() {
	auto world = PhysicsManager::GetSingleton()->world;
	collider = NewtonCreateBox(world, size.x, size.y, size.z, 0, nullptr);
}

void BoxColliderComponent::SetMassMatrix() {
	// TODO: Make members
	f32 mass = 1.;

	// http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetMassMatrix
	f32 Ixx = mass * (size.y*size.y + size.z*size.z) / 12;
	f32 Iyy = mass * (size.x*size.x + size.z*size.z) / 12;
	f32 Izz = mass * (size.x*size.x + size.y*size.y) / 12;

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

	collider = NewtonCreateCapsule(world, radius, height, 0, transform[0]);
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