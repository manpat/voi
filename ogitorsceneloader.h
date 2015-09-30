#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include "common.h"
#include "sceneloader.h"
#include <rapidxml.hpp>

struct App;

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

		EntityDef* entity;
		std::vector<Node> nodes;
	};

	std::vector<ResourceLocation> resourceLocations;
	std::vector<Node> nodes;

	~OgitorSceneLoader();

	void Load(const std::string& filename, App*) override;

protected:
	void ConstructScene(App*);

	std::vector<ResourceLocation> ParseResourceLocations(rapidxml::xml_node<>* node);
	std::vector<Node> ParseNodes(rapidxml::xml_node<>* node);
	EntityDef* ParseEntity(rapidxml::xml_node<>* node);

	vec3 ParseVec(rapidxml::xml_node<>*);
	quat ParseQuaternion(rapidxml::xml_node<>*);
	UserData ParseUserData(rapidxml::xml_node<>*);
};

#endif