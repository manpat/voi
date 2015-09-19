#include "entitymanager.h"
#include "component.h"
#include "entity.h"
#include "pool.h"
#include "app.h"

#include <algorithm>

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreEntity.h>

FramePool* Entity::messagePool = nullptr;

void Entity::Init(){
	components = {};
	children = {};
	parent = nullptr;
	userdata = {};
	ogreEntity = nullptr;
	ogreSceneNode = nullptr;
	// id set by entity manager

	enabled = true;
}

void Entity::Destroy(){
	for(auto c: components){
		c->OnDestroy();
		delete c;
	}

	// TODO: TEST

	// This will recurse and destroy all leaf nodes first
	//	Note that circular references will kill this
	for(auto e: children){
		EntityManager::instance->DestroyEntity(e);
	}

	if(ogreSceneNode){
		ogreSceneNode->removeAndDestroyAllChildren();
		App::instance->sceneManager->destroySceneNode(ogreSceneNode);
	}

	if(ogreEntity)
		App::instance->sceneManager->destroyEntity(ogreEntity);
}

void Entity::Update(){
	for(auto c: components){
		c->OnUpdate();
	}
}

void Entity::AddChild(Entity* e){
	if(!e) return;
	if(e->parent) throw "Tried to child an entity that already has a parent";

	children.push_back(e);
	e->parent = this;

	if(ogreSceneNode)
		ogreSceneNode->addChild(e->ogreSceneNode);
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

	if(ogreSceneNode)
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

	if(ogreSceneNode)
		ogreSceneNode->removeChild(e->ogreSceneNode);

	// Destroy it
	EntityManager::instance->DestroyEntity(e);
}

void Entity::AddComponent(Component* c){
	if(!c) return;
	if(c->entity) throw "Tried to add a component already attached to another entity";

	components.push_back(c);
	c->entity = this;
	c->enabled = true;
	c->OnInit();

	EntityManager::instance->newComponents.push_back(c);
}

void Entity::RemoveComponent(Component* c){
	if(!c) return;

	auto end = components.end();
	auto it = std::remove(components.begin(), end, c);

	if(it == end) return;

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

template<>
void Entity::SendMessage(const std::string& type){
	OpaqueType ot;

	for(auto c: components){
		c->OnMessage(type, ot);
	}
}
// SendMessage with arguments defined in entity.inl

const std::string& Entity::GetName() const {
	if(!ogreEntity) throw "Tried to get name of Entity with no Ogre::Entity";
	return ogreEntity->getName();
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
		ComponentB(int _x) : Component{this}, x(_x) {}
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
	auto e = emgr.CreateEntity();
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