#include <fstream>
#include <iostream>
#include <queue>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>

#include "sceneparser.h"
#include "portalmanager.h"
#include "app.h"

using namespace rapidxml;

static void error(const std::string& e){
	std::cerr << ("Error: " + e) << std::endl;
	//throw "SceneParser error";
}

SceneParser::~SceneParser(){
	std::queue<Node*> nodeQueue;
	for(auto& n: nodes)
		nodeQueue.push(&n);

	while(nodeQueue.size() > 0){
		auto& n = nodeQueue.front();
		nodeQueue.pop();

		for(auto& cn : n->nodes){
			nodeQueue.push(&cn);
		}

		if(n->entity){
			delete n->entity;
		}
	}
}

void SceneParser::Load(std::string filename, App* app) {
	std::fstream file(filename);
	if(!file) throw "File open failed";

	// Load document
	file.seekg(0, file.end);
	u64 len = file.tellg();
	file.seekg(0, file.beg);

	auto data = new char[len + 1l];
	memset(data, 0, len + 1l);
	file.read(data, len);
	file.close();

	// Parse xml
	xml_document<> doc;
	doc.parse<0>(data);

	// Construct tree
	auto node = doc.first_node("scene");
	resourceLocations = ParseResourceLocations(node->first_node("resourceLocations"));
	nodes = ParseNodes(node->first_node("nodes"));
	delete[] data;

	// Init resource locations
	for(auto& rl: resourceLocations){
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(rl.dir, rl.type);
	}
	// Ogre::ResourceGroupManager::getS3ingleton().addResourceLocation("GameData/Meshes", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Particles", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Construct scene
	ConstructScene(app);
}

void SceneParser::ConstructScene(App* app){
	auto rootNode = app->rootNode;

	struct NodeParentPair {
		Ogre::SceneNode* parent;
		Node* ndef;
	};
	std::queue<NodeParentPair> nodeQueue;

	if (nodes.size() == 0){
		error("Trying to construct empty scene");
		return;
	}

	for(auto& ndef: nodes){
		nodeQueue.push({rootNode, &ndef});
	}

	while(nodeQueue.size() > 0){
		auto& n = nodeQueue.front();
		nodeQueue.pop();
		
		auto& ndef = *n.ndef;
		auto node = n.parent->createChildSceneNode(ndef.name, ndef.position, ndef.rotation);
		node->setScale(ndef.scale);

		if(ndef.entity){
			// If entities were stored in a queue like nodes, then they could be
			//	iterated through to find portals
			auto& entdef = *ndef.entity;
			auto ent = app->sceneManager->createEntity(entdef.name, entdef.mesh);
			node->attachObject(ent);

			// Set layer
			auto layerStr = findin(entdef.userData, std::string("Layer"), std::string());
			if(layerStr.size() == 0){
				throw entdef.name + " is missing layer property";
			}
			auto layer = std::stol(layerStr);
			assert(layer < 10);

			// Test if contains portal
			ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+layer);

			if(findin(entdef.userData, std::string("IsPortal")) == "true"){
				auto dstlayerStr = findin(entdef.userData, std::string("DstLayer"), std::string("1"));
				auto dstlayer = std::stol(dstlayerStr);
				assert(dstlayer < 10);

				app->portalManager->AddPortal(ent, layer, dstlayer);
			}

			// Set user data
			auto& uob = ent->getUserObjectBindings();
			for(auto& pair: ndef.userData){
				uob.setUserAny(pair.first, Ogre::Any(pair.second));
			}
		}

		for(auto& child: ndef.nodes){
			nodeQueue.push({node, &child});
		}
	}
}

auto SceneParser::ParseResourceLocations(xml_node<>* node) -> std::vector<ResourceLocation>{
	std::vector<ResourceLocation> rls;

	if(!node) {
		error("No resourceLocations node in scene file");
		return rls;
	}

	for(auto rl = node->first_node("resourceLocation"); rl; rl = rl->next_sibling("resourceLocation")) {
		auto type = rl->first_attribute("type");
		auto dir = rl->first_attribute("name");

		if(!type || !dir) {
			error("Malformed resourceLocation");
			continue;
		}

		rls.push_back(ResourceLocation{
			std::string(type->value()),
			std::string(dir->value()),
		});
	} 

	return rls;
}

auto SceneParser::ParseNodes(xml_node<>* node) -> std::vector<Node> {
	std::vector<Node> nodes;
	if(!node) return nodes;

	for(auto n = node->first_node("node"); n; n = n->next_sibling("node")) {
		auto nameattr = n->first_attribute("name");
		auto posnode = n->first_node("position");
		auto rotnode = n->first_node("rotation");
		auto scalenode = n->first_node("scale");
		auto entnode = n->first_node("entity");
		auto udnode = n->first_node("userData");

		if(!nameattr || !posnode || !rotnode || !scalenode) {
			error("Malformed node");
			continue;
		}

		auto nn = Node{};
		nn.name = nameattr->value();
		nn.position = ParseVec(posnode);
		nn.rotation = ParseQuaternion(rotnode);
		nn.scale = ParseVec(scalenode);
		nn.entity = entnode?ParseEntity(entnode):nullptr;
		nn.nodes = ParseNodes(n);
		nn.userData = udnode?ParseUserData(udnode):UserData{};

		nodes.push_back(std::move(nn));
	}

	return nodes;
}

auto SceneParser::ParseEntity(xml_node<>* node) -> Entity* {
	auto nameattr = node->first_attribute("name");
	auto meshattr = node->first_attribute("meshFile");
	auto udnode = node->first_node("userData");

	if(!nameattr || !meshattr) {
		error("Malformed entity");
		return nullptr;
	}
	auto e = new Entity;
	e->name = nameattr->value();
	e->mesh = meshattr->value();
	e->userData = udnode?ParseUserData(udnode):UserData{};

	for(auto se = node->first_node("subentity"); se; se=se->next_sibling("subentity")){
		auto index = se->first_attribute("index");
		auto material = se->first_attribute("materialName");

		if(!index || !material){
			error("Malformed SubEntity");
			continue;
		}

		e->subEntities.push_back(SubEntity{
			(unsigned) std::strtol(index->value(), nullptr, 10),
			material->value()
		});
	}

	return e;
}

vec3 SceneParser::ParseVec(xml_node<>* node){
	auto xn = node->first_attribute("x");
	auto yn = node->first_attribute("y");
	auto zn = node->first_attribute("z");

	if(!xn || !yn || !zn) {
		error("Fucked vector");
		return vec3();
	}

	auto x = strtof(xn->value(), nullptr);
	auto y = strtof(yn->value(), nullptr);
	auto z = strtof(zn->value(), nullptr);

	return vec3(x,y,z);
}

quat SceneParser::ParseQuaternion(xml_node<>* node){
	auto xn = node->first_attribute("qx");
	auto yn = node->first_attribute("qy");
	auto zn = node->first_attribute("qz");
	auto wn = node->first_attribute("qw");

	if(!xn || !yn || !zn || !wn) {
		error("Fucked quaternion");
		return quat();
	}

	auto x = strtof(xn->value(), nullptr);
	auto y = strtof(yn->value(), nullptr);
	auto z = strtof(zn->value(), nullptr);
	auto w = strtof(wn->value(), nullptr);

	return quat(w,x,y,z);
}

auto SceneParser::ParseUserData(xml_node<>* node) -> UserData {
	UserData ud;

	for(auto n = node->first_node("property"); n; 
		n = n->next_sibling("property")){

		auto name = n->first_attribute("name");
		auto data = n->first_attribute("data");

		if(!name || !data){
			error("Malformed userdata");
			continue;
		}

		ud[name->value()] = data->value();
	}

	return ud;
}
