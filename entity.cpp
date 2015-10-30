#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "entitymanager.h"
#include "component.h"
#include "entity.h"
#include "pool.h"
#include "app.h"

#include <algorithm>

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreEntity.h>

#undef SendMessage // TODO: Fix properly on Windows

FramePool* Entity::messagePool = nullptr;

void Entity::Init(){
	components = {};
	children = {};
	parent = nullptr;
	//userdata = {}; // TODO: Fix properly in VS2013
	ogreEntity = nullptr;
	ogreSceneNode = nullptr;
	// id set by entity manager
	layer = 0;

	enabled = true;
}

void Entity::Destroy(){
	for(auto it = components.begin(); it != components.end(); ++it){
		auto c = *it;
		c->OnDestroy();
		delete c;
	}
	components.clear();
	children.clear();

	if(ogreSceneNode){
		// ogreSceneNode->removeAndDestroyAllChildren();

		// Don't destroy children because they should be 
		//	managed by other entities
		ogreSceneNode->removeAllChildren();
		App::GetSingleton()->sceneManager->destroySceneNode(ogreSceneNode);
	}

	if(ogreEntity)
		App::GetSingleton()->sceneManager->destroyEntity(ogreEntity);
}

void Entity::DestroyRecurse(){
	for(auto it = components.begin(); it != components.end(); ++it){
		auto c = *it;
		c->OnDestroy();
		delete c;
	}
	components.clear();

	// TODO: TEST

	// This will recurse and destroy all leaf nodes first
	//	Note that circular references will kill this
	for(auto e = children.begin(); e != children.end(); ++e){
		EntityManager::GetSingleton()->DestroyEntity(*e);
	}
	children.clear();

	if(ogreSceneNode){
		// ogreSceneNode->removeAndDestroyAllChildren();

		// Don't destroy children because they should be
		//	managed by other entities
		ogreSceneNode->removeAllChildren();
		App::GetSingleton()->sceneManager->destroySceneNode(ogreSceneNode);
	}

	if(ogreEntity)
		App::GetSingleton()->sceneManager->destroyEntity(ogreEntity);
}

void Entity::Update(){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnUpdate();
	}
}

void Entity::LateUpdate(){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnLateUpdate();
	}
}

void Entity::AddChild(Entity* e){
	if(!e) return;
	if(e->parent) throw "Tried to child an entity that already has a parent";

	children.push_back(e);
	e->parent = this;

	if(ogreSceneNode && e->ogreSceneNode){
		auto p = e->ogreSceneNode->getParentSceneNode();

		// Already parented
		if(p == ogreSceneNode) return;

		// parent not null, and is not this so orphan
		if(p && p != ogreSceneNode){
			// Remove previous parent
			p->removeChild(e->ogreSceneNode);
		}

		ogreSceneNode->addChild(e->ogreSceneNode);
	}
}

void Entity::RemoveChild(Entity* e){
	if(!e) return;

	// Remove e from children
	auto end = children.end();
	auto it = std::remove(children.begin(), end, e);

	// If it wasn't a child, give up
	if(it == end) return;

	children.erase(it, end);

	// Reset parent if e is actually a child
	e->parent = nullptr;

	if(ogreSceneNode && e->ogreSceneNode)
		ogreSceneNode->removeChild(e->ogreSceneNode);
}

void Entity::DestroyChild(Entity* e){
	if(!e) return;

	// Remove e from children
	auto end = children.end();
	auto it = std::remove(children.begin(), end, e);

	// If it wasn't a child, give up
	if(it == end) return;

	children.erase(it, end);

	if(ogreSceneNode && e->ogreSceneNode)
		ogreSceneNode->removeChild(e->ogreSceneNode);

	// Destroy it
	EntityManager::GetSingleton()->DestroyEntity(e);
}

void Entity::OrphanSelf(){
	if(!parent) return;
	parent->RemoveChild(this);
}

void Entity::AddComponent(Component* c){
	if(!c) return;
	if(c->entity) throw "Tried to add a component already attached to another entity";

	if(c->IsType<ColliderComponent>()){
		collider = static_cast<ColliderComponent*>(c);
	}

	c->entity = this;
	c->enabled = true;
	c->OnInit();

	components.push_back(c);
	EntityManager::GetSingleton()->newComponents.push(c);
}

void Entity::RemoveComponent(Component* c){
	if(!c) return;

	auto end = components.end();
	auto it = std::remove(components.begin(), end, c);

	if(it == end) return;

	if(collider == c){
		collider = nullptr;
	}

	components.erase(it, end);
	c->OnRemove();
	c->entity = nullptr;
}

void Entity::DestroyComponent(Component* c){
	if(!c) return;

	RemoveComponent(c);
	c->OnDestroy();
	delete c;
}

void Entity::SendMessage(const std::string& type){
	OpaqueType ot;

	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnMessage(type, ot);
	}
}
// SendMessage with arguments defined in entity.inl

const std::string& Entity::GetName() const {
	static std::string errorName {"<unnamed entity>"};
	if(!ogreSceneNode) return errorName;
	return ogreSceneNode->getName();
}
const vec3& Entity::GetPosition() const {
	if(!ogreSceneNode) throw "Tried to get local position of Entity with no Ogre::SceneNode";
	return ogreSceneNode->getPosition();
}
const vec3& Entity::GetGlobalPosition() const {
	if(!ogreSceneNode) throw "Tried to get position of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getDerivedPosition();
}
const quat& Entity::GetOrientation() const {
	if(!ogreSceneNode) throw "Tried to get orientation of Entity with no Ogre::SceneNode";
	return ogreSceneNode->getOrientation();
}
const quat& Entity::GetGlobalOrientation() const {
	if(!ogreSceneNode) throw "Tried to get orientation of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getDerivedOrientation();
}
vec3 Entity::GetRight() const {
	if (!ogreSceneNode) throw "Tried to get X axis of Entity with no Ogre::SceneNode";
	return ogreSceneNode->getOrientation().xAxis();
}
vec3 Entity::GetUp() const {
	if (!ogreSceneNode) throw "Tried to get Y axis of Entity with no Ogre::SceneNode";
	return ogreSceneNode->getOrientation().yAxis();
}
vec3 Entity::GetForward() const {
	if (!ogreSceneNode) throw "Tried to get Z axis of Entity with no Ogre::SceneNode";
	return -ogreSceneNode->getOrientation().zAxis();
}
vec3 Entity::GetGlobalRight() const {
	if (!ogreSceneNode) throw "Tried to get X axis of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getDerivedOrientation().xAxis();
}
vec3 Entity::GetGlobalUp() const {
	if (!ogreSceneNode) throw "Tried to get Y axis of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getDerivedOrientation().yAxis();
}
vec3 Entity::GetGlobalForward() const {
	if (!ogreSceneNode) throw "Tried to get Z axis of Entity with no Ogre::SceneNode";
	return -ogreSceneNode->_getDerivedOrientation().zAxis();
}
const vec3& Entity::GetScale() const {
	if(!ogreSceneNode) throw "Tried to get scale of Entity with no Ogre::SceneNode";
	return ogreSceneNode->getScale();
}
const vec3& Entity::GetGlobalScale() const {
	if(!ogreSceneNode) throw "Tried to get scale of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getDerivedScale();
}
const mat4& Entity::GetFullTransform() const {
	if(!ogreSceneNode) throw "Tried to get transform of Entity with no Ogre::SceneNode";
	return ogreSceneNode->_getFullTransform();
}


void Entity::SetPosition(const vec3& n) {
	if(!ogreSceneNode) throw "Tried to set local position of Entity with no Ogre::SceneNode";
	ogreSceneNode->setPosition(n);
}
void Entity::SetGlobalPosition(const vec3& n) {
	if(!ogreSceneNode) throw "Tried to set position of Entity with no Ogre::SceneNode";
	ogreSceneNode->_setDerivedPosition(n);
}
void Entity::SetOrientation(const quat& n) {
	if(!ogreSceneNode) throw "Tried to set orientation of Entity with no Ogre::SceneNode";
	ogreSceneNode->setOrientation(n);
}
void Entity::SetGlobalOrientation(const quat& n) {
	if(!ogreSceneNode) throw "Tried to set orientation of Entity with no Ogre::SceneNode";
	ogreSceneNode->_setDerivedOrientation(n);
}
void Entity::SetScale(const vec3& n) {
	if(!ogreSceneNode) throw "Tried to set scale of Entity with no Ogre::SceneNode";
	ogreSceneNode->setScale(n);
}
// void Entity::SetGlobalScale(const vec3& n) {
// 	if(!ogreSceneNode) throw "Tried to set scale of Entity with no Ogre::SceneNode";
// 	ogreSceneNode->_setDerivedScale(n);
// }
// void Entity::SetFullTransform(const mat4& n) {
// 	if(!ogreSceneNode) throw "Tried to set transform of Entity with no Ogre::SceneNode";
// 	ogreSceneNode->_setFullTransform(n);
// }

void Entity::Translate(const vec3& diff) {
	if (!ogreSceneNode) throw "Tried to translate Entity with no Ogre::SceneNode";
	ogreSceneNode->translate(diff);
}
void Entity::Lerp(const vec3& from, const vec3& to, float percent) {
	if (!ogreSceneNode) throw "Tried to lerp Entity with no Ogre::SceneNode";
	auto dist = (to - from) * percent;
	ogreSceneNode->translate(dist);
}
void Entity::Rotate(const quat& rot, bool worldSpace) {
	if (!ogreSceneNode) throw "Tried to rotate Entity with no Ogre::SceneNode";
	ogreSceneNode->rotate(rot, worldSpace ? Ogre::Node::TS_WORLD : Ogre::Node::TS_LOCAL);
}
void Entity::Slerp(const quat& from, const quat& to, float t) {
	if (!ogreSceneNode) throw "Tried to slerp Entity with no Ogre::SceneNode";
	ogreSceneNode->rotate(quat::Slerp(t, from, to, true));
}
void Entity::LookAt(const vec3& pos, bool worldSpace) {
	if (!ogreSceneNode) throw "Tried to \"look at\" using an Entity with no Ogre::SceneNode";
	ogreSceneNode->lookAt(pos, worldSpace ? Ogre::Node::TS_WORLD : Ogre::Node::TS_LOCAL);
}

void Entity::SetLayer(s32 l, bool hidden) {
	layer = l;

	if(ogreEntity){
		ogreEntity->setRenderQueueGroup((hidden?RENDER_QUEUE_HIDDEN:RENDER_QUEUE_LAYER) + (u8)layer);
		
		// HACK: Remove as soon as possible
		//ogreEntity->getMesh()->_setBounds(Ogre::AxisAlignedBox(Ogre::AxisAlignedBox::EXTENT_INFINITE));
	}

	if(collider){
		// Last 5 bits reserved for layer stuff
		auto cg = collider->collisionGroups & ~((1<<6) - 1);
		collider->collisionGroups = cg | 1<<layer;
		collider->Refilter();
	}

	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnLayerChange();
	}
}

void Entity::SetVisible(bool v){
	if(!ogreEntity) throw "Tried to set visibility of entity with no Ogre::Entity";
	ogreEntity->setVisible(v);
}

void Entity::OnCollisionEnter(ColliderComponent* oc){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnCollisionEnter(oc);
	}
}
void Entity::OnCollisionLeave(ColliderComponent* oc){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnCollisionLeave(oc);
	}
}
void Entity::OnTriggerEnter(ColliderComponent* oc){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnTriggerEnter(oc);
	}
}
void Entity::OnTriggerLeave(ColliderComponent* oc){
	for(auto c = components.begin(); c != components.end(); ++c){
		(*c)->OnTriggerLeave(oc);
	}
}

/*

	88        88             88            888888888888
	88        88             ""   ,d            88                        ,d
	88        88                  88            88                        88
	88        88 8b,dPPYba,  88 MM88MMM         88  ,adPPYba, ,adPPYba, MM88MMM
	88        88 88P'   `"8a 88   88            88 a8P_____88 I8[    ""   88
	88        88 88       88 88   88            88 8PP"""""""  `"Y8ba,    88
	Y8a.    .a8P 88       88 88   88,           88 "8b,   ,aa aa    ]8I   88,
	 `"Y8888Y"'  88       88 88   "Y888         88  `"Ybbd8"' `"YbbdP"'   "Y888


*/
void unittest_Entity(){
	struct ComponentA : Component {
		ComponentA(int _x) : Component{this}, x(_x) {}
		~ComponentA()    { std::cout << "A " << id << " Destructor\n"; }
		void OnInit()    { std::cout << "A " << id << " OnInit\n"; }
		void OnAwake()   { std::cout << "A " << id << " OnAwake\n"; }
		void OnRemove()  { std::cout << "A " << id << " OnRemove\n"; }
		void OnDestroy() { std::cout << "A " << id << " OnDestroy\n"; }
		void OnUpdate()  { std::cout << "A " << id << " OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "A " << id << " OnMessage " << type << "\t";
			auto data = ot.Get<std::tuple<int,int,int,int>>(false);

			if(!data) {
				std::cout << "A Unexpected message data type " << ot.name << "\n";
				return;
			}

			std::cout << "A ";
			std::cout << std::get<0>(*data) << " ";
			std::cout << std::get<1>(*data) << " ";
			std::cout << std::get<2>(*data) << " ";
			std::cout << std::get<3>(*data) << " ";
			std::cout << x << "\n";
		}

		int x;
	};
	struct ComponentB : Component {
		ComponentB(int _x) : Component{this}, x((float)_x) {}
		void OnInit()    { std::cout << "B " << id << " OnInit\n"; }
		void OnAwake()   { std::cout << "B " << id << " OnAwake\n"; }
		void OnDestroy() { std::cout << "B " << id << " OnDestroy\n"; }
		void OnUpdate()  { std::cout << "B " << id << " OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "B " << id << " OnMessage " << type << "\t";

			if(type == "comp"){
				auto c = *ot.Get<ComponentA*>(/* default true: will throw on type mismatch */);
				std::cout << "B recieved a ComponentA with x: " << c->x << "\n";
			}else{
				std::cout << "B recieved type " << ot.name << "\n";
			}
		}

		float x;
	};
	struct ComponentC : Component {
		ComponentC() : Component{this} {}
		void OnMessage(const std::string& type, const OpaqueType&){
			std::cout << "C " << id << " OnMessage " << type << "\n";
		}
	};

	EntityManager emgr {};
	auto e = emgr.CreateEntity("blah");
	auto c1 = e->AddComponent<ComponentA>(5);
	auto c2 = e->AddComponent<ComponentA>(6);
	auto c3 = e->AddComponent<ComponentB>(7);

	std::cout << std::boolalpha;
	std::cout << "c1 Is ComponentA? " << c1->IsType<ComponentA>() << "\n";
	std::cout << "c1 Is ComponentB? " << c1->IsType<ComponentB>() << "\n";
	std::cout << "c3 Is ComponentA? " << c3->IsType<ComponentA>() << "\n";
	std::cout << "c3 Is ComponentB? " << c3->IsType<ComponentB>() << "\n";

	std::cout << "c1 SameType as c2? " << c1->SameType(c2) << "\n";
	std::cout << "c1 SameType as c3? " << c1->SameType(c3) << "\n";

	std::cout << "\n";
	std::cout << "e->FindComponent<ComponentA> == c1? " << (e->FindComponent<ComponentA>() == (Component*)c1) << "\n";
	std::cout << "e->FindComponent<ComponentB> != c2? " << (e->FindComponent<ComponentB>() != (Component*)c2) << "\n";
	std::cout << "e->FindComponent<ComponentB> == c3? " << (e->FindComponent<ComponentB>() == (Component*)c3) << "\n";

	std::cout << "\nUpdate\n";
	emgr.Update();
	e->SendMessage("four", 1, 2, 3, 4);
	e->SendMessage("one ", 1.f);
	e->SendMessage("null");
	e->SendMessage("comp", c1);

	std::cout << "\nUpdate\n";
	emgr.Update();
	e->RemoveComponent(c1);
	e->DestroyComponent(c2);
	e->AddComponent<ComponentC>();
	// c2 is invalid here
	// c1 is still valid
	// e has two active components: [c3, ComponentC{}]

	std::cout << "\nUpdate\n";
	emgr.Update();
	e->AddComponent(c1);
	e->DestroyComponent(c3);
	e->SendMessageRecurse("test", 'a', "123");
	// c1 is the only valid component here

	std::cout << "\nUpdate\n";
	emgr.Update();
}