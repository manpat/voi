#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_map>

#include "common.h"

// Collection of Components
// Collection of Children
// Userdata
// Parent
// Id

// Updatable
// Transform operations (modifications to ogre transform)
// Component Queries/Operations
// Message Distribution

namespace Ogre {
	class Entity;
	class SceneNode;
}

struct Component;
struct FramePool;
struct PhysicsManager;
struct ColliderComponent;

struct Entity {
	friend PhysicsManager;
	static FramePool* messagePool;

	// Vector of components, owned by this entity
	std::vector<Component*> components;

	// Vector of children entities, owned by entity manager
	std::vector<Entity*> children;

	// Parent entity, null if root entity
	Entity* parent;

	// Collider pointer for caching purposes, since most entities
	//	will have one. It assumes each entity will only ever have
	//	one collider
	ColliderComponent* collider;

	// Stores all the extra userdata stored in the scene file
	std::unordered_map<std::string, std::string> userdata; // TODO: Make better

	// Both of these are optional but required if the entity is to have
	//	a transform, name or collider
	Ogre::Entity* ogreEntity;
	Ogre::SceneNode* ogreSceneNode;

	// Unique identifier, id 0 is invalid
	//	id 0 can be used for pooling
	u32 id;

	// The layer than this entity exists in
	//	NOTE: setting directly will do nothing
	s32 layer;

	// Whether or not this should recieve updates
	bool enabled;

	// Init resets Entity to default state and cleans up previous data.
	// 	To help with pooling
	void Init();

	// Destroy cleans up components
	void Destroy();

	// Destroy cleans up components and children
	void DestroyRecurse();

	// Update updates all attached and active components
	void Update();

	// LateUpdate late updates all attached and active components
	void LateUpdate();

	// AddChild attaches a child entity
	void AddChild(Entity*);
	// RemoveChild detaches a child entity but does not destroy it
	void RemoveChild(Entity*);
	// DestroyChild detaches and destroys a child entity
	void DestroyChild(Entity*);
	// OrphanSelf detaches self from parent
	void OrphanSelf();

	// AddComponent<C,A...> constructs a component with type C with arguments
	//	of types A... or void. Returns new component
	template<class C, class... A>
	C* AddComponent(A...);
	// AddComponent attaches an already constructed component
	void AddComponent(Component*);
	// RemoveComponent removes a component if it is attached but does not destroy it
	void RemoveComponent(Component*);
	// DestroyComponent removes a component and deletes it if it is attached
	void DestroyComponent(Component*);

	// FindComponent returns the first component of type C or nullptr
	template<class C>
	C* FindComponent();

	// SendMessage broadcasts a message to attached components
	// If zero arguments are given, a nullptr packet is sent to OnMessage
	// If one argument is given, a copy of that argument is sent to OnMessage
	// If more than one argument is given, a std::tuple of all arguments is sent to OnMessage
	//
	// For example: entity->SendMessage("messageType", 1, 2, 3.f, 'a'); sends the
	//	packet std::tuple<s32, s32, f32, char>
	template<class... A>
	void SendMessage(const std::string&, A... arguments);
	void SendMessage(const std::string&);

	// SendMessageRecurse broadcasts a message to attached components
	//	and child entities
	// See SendMessage
	template<class... A>
	void SendMessageRecurse(const std::string&, A...);

	const std::string& GetName() const;
	const vec3& GetPosition() const;
	const vec3& GetGlobalPosition() const;
	const quat& GetOrientation() const;
	const quat& GetGlobalOrientation() const;
	vec3 GetRight() const;
	vec3 GetUp() const;
	vec3 GetForward() const;
	vec3 GetGlobalRight() const;
	vec3 GetGlobalUp() const;
	vec3 GetGlobalForward() const;
	const vec3& GetScale() const;
	const vec3& GetGlobalScale() const;
	const mat4& GetFullTransform() const;
	Ogre::Plane GetWorldPlaneFromMesh() const;

	void SetPosition(const vec3&);
	void SetGlobalPosition(const vec3&);
	void SetOrientation(const quat&);
	void SetGlobalOrientation(const quat&);
	void SetScale(const vec3&);

	// Move by this value
	void Translate(const vec3&);
	// Move from from point a to b, by progress
	void Lerp(const vec3& a, const vec3& b, float progress);
	// Rotate by this value
	void Rotate(const quat&, bool worldSpace = true);
	// Rotate from point a to b, by progress
	void Slerp(const quat& a, const quat& b, float progress);
	// Rotate to look at this position
	void LookAt(const vec3& pos, bool worldSpace = true);

	// TODO: Add coordinate conversion

	void SetLayer(s32, bool=false);
	void SetVisible(bool);

protected:
	void OnCollisionEnter(ColliderComponent*);
	void OnCollisionLeave(ColliderComponent*);
	void OnTriggerEnter(ColliderComponent*);
	void OnTriggerLeave(ColliderComponent*);
};

#include "entity.inl"

#endif