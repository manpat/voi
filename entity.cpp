#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "entitymanager.h"
#include "component.h"
#include "meshinfo.h"
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
	for (auto it = components.begin(); it != components.end();) {
		auto c = *it;

		if (!c) {
			++it;
			continue;
		}

		c->OnDestroy();
		it = components.erase(it);

		delete c;
		c = nullptr;
	}

	//components.clear();
	assert(components.empty());

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

Ogre::Plane Entity::GetWorldPlaneFromMesh() const {
	auto mesh = GetOgreSubMeshVertices(*ogreEntity->getMesh()->getSubMeshIterator().begin());

	if (mesh.size() >= 3) {
		// Calculate normal from mesh vertices
		auto p1 = mesh[1] - mesh[0];
		auto p2 = mesh[2] - mesh[0];

		auto normal = p2.crossProduct(p1);
		normal.normalise();

		// p is a point on the plane
		auto p = vec3::ZERO;

		// Determine that point from the average of all points for later on when aligning to orientation
		for (u32 v = 0; v < mesh.size(); ++v) {
			p += mesh[v];
		}

		p /= (f32)mesh.size();

		// Offset p by the node position
		auto node = ogreEntity->getParentSceneNode();
		p += node->_getDerivedPosition();

		// n is the normal or unit vector perpendicular to the plane
		auto n = ogreEntity->getParentSceneNode()->_getDerivedOrientation() * normal;
		auto l = p.dotProduct(n);

		// l is length
		return Ogre::Plane(n, l);
	}

	return Ogre::Plane();
}

void Entity::SetPosition(const vec3& n) {
	if(!ogreSceneNode) throw "Tried to set local position of Entity with no Ogre::SceneNode";
	ogreSceneNode->setPosition(n);
}
void Entity::SetGlobalPosition(const vec3& n) {
	if(!ogreSceneNode) throw "Tried to set position of Entity with no Ogre::SceneNode";

	if (collider != nullptr) {
		collider->SetPosition(n);
	} else {
		ogreSceneNode->_setDerivedPosition(n);
	}
}
void Entity::SetOrientation(const quat& n) {
	if(!ogreSceneNode) throw "Tried to set orientation of Entity with no Ogre::SceneNode";
	ogreSceneNode->setOrientation(n);
}
void Entity::SetGlobalOrientation(const quat& n) {
	if(!ogreSceneNode) throw "Tried to set orientation of Entity with no Ogre::SceneNode";

	if (collider != nullptr) {
		collider->SetOrientation(n);
	} else {
		ogreSceneNode->_setDerivedOrientation(n);
	}
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
	}

	if(collider){
		// Last 5 bits reserved for layer stuff
		auto cg = collider->collisionGroups & ~((1<<6) - 1);
		collider->collisionGroups = cg | 1<<layer;
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