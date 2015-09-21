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
	ent->SetPosition(position);

	// std::cout << "Entity " << ent->id << " moved" << std::endl;
}

extern "C" void ApplyGravityCallback(const NewtonBody* body, f32 dt, int){
	auto comp = static_cast<Component*>(NewtonBodyGetUserData(body));
	// auto ent = comp->entity;

	// TODO: This could be factored into a switch 
	//	when there are more collider types
	// Alternatively, a base Collider component could be used instead
	if(comp->IsType<BoxColliderComponent>()){
		auto boxc = static_cast<BoxColliderComponent*>(comp);
		NewtonBodyAddForce(body, &boxc->force.x);
		boxc->force = vec3::ZERO;
	}
}

void BoxColliderComponent::OnInit() {
	auto world = PhysicsManager::GetSingleton()->world;
	auto omat = entity->GetFullTransform();

	f32 nmat[16];
	OgreMatToF32Array(omat, nmat);

	collider = NewtonCreateBox(world, 1, 1, 1, 0, nullptr);
	body = NewtonCreateBody(world, collider, nmat);

	// Zero mass is a static object
	f32 mass = 1.;
	f32 lx = 1.;
	f32 ly = 1.;
	f32 lz = 1.;

	// http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetMassMatrix
	f32 Ixx = mass * (ly*ly + lz*lz) / 12;
	f32 Iyy = mass * (lx*lx + lz*lz) / 12;
	f32 Izz = mass * (lx*lx + ly*ly) / 12;
	NewtonBodySetMassMatrix(body, mass, Ixx, Iyy, Izz);

	NewtonBodySetUserData(body, this);
	NewtonBodySetTransformCallback(body, SetTransformCallback);
	NewtonBodySetForceAndTorqueCallback(body, ApplyGravityCallback);
}

void BoxColliderComponent::OnDestroy() {

}