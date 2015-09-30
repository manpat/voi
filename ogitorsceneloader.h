#ifndef OGITOR_SCENE_LOADER_H
#define OGITOR_SCENE_LOADER_H

#include "common.h"
#include "sceneloader.h"
#include <rapidxml.hpp>

struct OgitorSceneLoader : SceneLoaderInterface {
protected:
	using UserData = std::map<std::string, std::string>;

	struct ResourceLocation {
		std::string type;
		std::string dir;
	};

	struct Object {
		std::string name;
		UserData userData;
	};

	struct SubEntityDef {
		unsigned index;
		std::string materialName;
	};

	struct EntityDef : Object {
		std::string mesh;
		std::vector<SubEntityDef> subEntities;
	};

public:
	struct Node : Object {
		quat rotation;
		vec3 position;
		vec3 scale;

		std::shared_ptr<EntityDef> entity;
		std::vector<Node> nodes;
	};

	std::vector<ResourceLocation> resourceLocations;
	std::vector<Node> nodes;

	void Load(const std::string& filename, App*) override;

protected:
	void ConstructScene(App*);

	std::vector<ResourceLocation> ParseResourceLocations(rapidxml::xml_node<>* node);
	std::vector<Node> ParseNodes(rapidxml::xml_node<>* node);
	std::shared_ptr<EntityDef> ParseEntity(rapidxml::xml_node<>* node);

	vec3 ParseVec(rapidxml::xml_node<>*);
	quat ParseQuaternion(rapidxml::xml_node<>*);
	UserData ParseUserData(rapidxml::xml_node<>*);
};

#endif