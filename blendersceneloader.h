#ifndef BLENDER_SCENE_LOADER_H
#define BLENDER_SCENE_LOADER_H

#include "common.h"
#include "sceneloader.h"
#include <rapidxml.hpp>

struct BlenderSceneLoader : SceneLoaderInterface {
protected:
	using UserData = std::unordered_map<std::string, std::string>;

	struct Object {
		std::string name;
		UserData userData;
	};

	enum class ColliderType {
		Box, Capsule, Sphere, 
		Cylinder, Cone, 
		ConvexHull, Mesh
	};

	enum class PhysicsType {
		None, Static, Dynamic
	};

	struct EntityDef : Object {
		std::string mesh;
		vec3 bounds;
		PhysicsType physicsType;
		ColliderType colliderType;
	};

public:
	struct Node : Object {
		quat rotation;
		vec3 position;
		vec3 scale;

		std::shared_ptr<EntityDef> entity;
		std::vector<Node> nodes;
	};

	std::vector<Node> nodes;

	void Load(const std::string& path, App*) override;

protected:
	void ConstructScene(App*);
	void DestroyScene(App*);

	std::vector<Node> ParseNodes(rapidxml::xml_node<>* node);
	std::shared_ptr<EntityDef> ParseEntity(rapidxml::xml_node<>* node);

	vec3 ParseVec(rapidxml::xml_node<>*);
	quat ParseQuaternion(rapidxml::xml_node<>*);
	UserData ParseUserData(rapidxml::xml_node<>*);
};

#endif