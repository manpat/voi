#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include "common.h"
#include <rapidxml.hpp>

namespace Ogre {
	class SceneManager;
}

class SceneParser {
	using UserData = std::map<std::string, std::string>;

	struct ResourceLocation {
		std::string type;
		std::string dir;
	};

	struct Object {
		std::string name;
		UserData userData;
	};

	struct SubEntity {
		unsigned index;
		std::string materialName;
	};

	struct Entity : Object {
		std::string mesh;
		std::vector<SubEntity> subEntities;
	};

public:
	struct Node : Object {
		quat rotation;
		vec3 position;
		vec3 scale;

		Entity* entity;
		std::vector<Node> nodes;
	};

	std::vector<ResourceLocation> resourceLocations;
	std::vector<Node> nodes;

	~SceneParser();

	void Load(std::string filename, Ogre::SceneManager*);

protected:
	void ConstructScene(Ogre::SceneManager*);

	std::vector<ResourceLocation> ParseResourceLocations(rapidxml::xml_node<>* node);
	std::vector<Node> ParseNodes(rapidxml::xml_node<>* node);
	Entity* ParseEntity(rapidxml::xml_node<>* node);

	vec3 ParseVec(rapidxml::xml_node<>*);
	quat ParseQuaternion(rapidxml::xml_node<>*);
	UserData ParseUserData(rapidxml::xml_node<>*);
};

#endif