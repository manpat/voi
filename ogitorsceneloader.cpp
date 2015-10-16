#include <iostream>
#include <fstream>
#include <queue>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>

#include "layerrenderingmanager.h"
#include "ogitorsceneloader.h"
#include "portalmanager.h"
#include "entitymanager.h"
#include "entity.h"
#include "app.h"

using namespace rapidxml;

static void error(const std::string& e){
	std::cerr << ("!!! Error: " + e) << std::endl;
	//throw "OgitorSceneLoader error";
}

void OgitorSceneLoader::Load(const std::string& filename, App* app) {
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
	// Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Meshes", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Particles", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Construct scene
	ConstructScene(app);
}

void OgitorSceneLoader::ConstructScene(App* app){
	auto rootNode = app->rootNode;
	auto entMgr = app->entityManager;

	struct NodeParentPair {
		Ogre::SceneNode* parentNode;
		Entity* parent;
		Node* ndef;
	};
	std::queue<NodeParentPair> nodeQueue;

	if (nodes.size() == 0){
		error("Trying to construct empty scene");
		return;
	}

	for(auto& ndef: nodes){
		nodeQueue.push({rootNode, nullptr, &ndef});
	}

	while(nodeQueue.size() > 0){
		auto& n = nodeQueue.front();
		nodeQueue.pop();
		
		auto& ndef = *n.ndef;
		auto node = n.parentNode->createChildSceneNode(ndef.name, ndef.position, ndef.rotation);
		node->setScale(ndef.scale);

		Entity* ent = nullptr;

		if(ndef.entity){
			// If entities were stored in a queue like nodes, then they could be
			//	iterated through to find portals
			auto& entdef = *ndef.entity;
			auto ogreent = app->sceneManager->createEntity(entdef.name, entdef.mesh);
			node->attachObject(ogreent);

			ent = entMgr->CreateEntity(node);
			ent->ogreEntity = ogreent;
			if(n.parent){
				n.parent->AddChild(ent);
			}

			// Set layer
			auto layerStr = findin(entdef.userData, std::string{"Layer"}, std::string{});
			if(layerStr.size() == 0){
				throw entdef.name + " is missing layer property";
			}
			auto layer = std::stol(layerStr);
			assert(layer < 10);

			// Test if contains portal
			ogreent->setRenderQueueGroup(RENDER_QUEUE_LAYER + (u8)layer);
			if(findin(entdef.userData, std::string("IsPortal")) == "true"){
				auto dstlayerStr = findin(entdef.userData, std::string{"DstLayer"}, std::string{"1"});
				auto dstlayer = std::stol(dstlayerStr);
				assert(dstlayer < 10);

				// TODO: Can't be fucked
				// app->portalManager->AddPortal(ogreent, layer, dstlayer);
			}

			// Set user data
			auto& uob = ogreent->getUserObjectBindings();
			uob.setUserAny(Ogre::Any{ent});

			for(auto& pair: ndef.userData){
				// uob.setUserAny(pair.first, Ogre::Any(pair.second));
				ent->userdata[pair.first] = pair.second;
			}
		}

		for(auto& child: ndef.nodes){
			nodeQueue.push({node, ent, &child});
		}
	}
}

auto OgitorSceneLoader::ParseResourceLocations(xml_node<>* node) -> std::vector<ResourceLocation>{
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

auto OgitorSceneLoader::ParseNodes(xml_node<>* node) -> std::vector<Node> {
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

auto OgitorSceneLoader::ParseEntity(xml_node<>* node) -> std::shared_ptr<EntityDef> {
	auto nameattr = node->first_attribute("name");
	auto meshattr = node->first_attribute("meshFile");
	auto udnode = node->first_node("userData");

	if(!nameattr || !meshattr) {
		error("Malformed entity");
		return nullptr;
	}
	auto e = std::make_shared<EntityDef>();
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

		e->subEntities.push_back(SubEntityDef{
			(unsigned) std::strtol(index->value(), nullptr, 10),
			material->value()
		});
	}

	return e;
}

vec3 OgitorSceneLoader::ParseVec(xml_node<>* node){
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

quat OgitorSceneLoader::ParseQuaternion(xml_node<>* node){
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

auto OgitorSceneLoader::ParseUserData(xml_node<>* node) -> UserData {
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
