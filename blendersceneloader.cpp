#include "blendersceneloader.h"

#include <iostream>
#include <fstream>
#include <queue>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>

#include "physicsmanager.h"
#include "portalmanager.h"
#include "entitymanager.h"
#include "doorcomponent.h"
#include "interactable.h"
#include "entity.h"
#include "app.h"

using namespace rapidxml;

static void error(const std::string& e){
	std::cerr << ("!!! Error: " + e) << std::endl;
	throw "BlenderSceneLoader error";
}

void BlenderSceneLoader::Load(const std::string& filename, App* app) {
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
	if(!node) {
		error("Scene file missing root scene node");
		return;
	}

	nodes = ParseNodes(node->first_node("nodes"));
	delete[] data;

	// Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Meshes", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Particles", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Construct scene
	ConstructScene(app);
}

void BlenderSceneLoader::ConstructScene(App* app){
	auto rootNode = app->rootNode;
	auto entMgr = app->entityManager;
	// auto physMgr = app->physicsManager;

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
		auto n = nodeQueue.front();
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

			ent = entMgr->CreateEntity();
			ent->ogreEntity = ogreent;
			ent->ogreSceneNode = node;
			if(n.parent){
				n.parent->AddChild(ent);
			}

			UserData& userdata = ent->userdata;

			for(auto& pair: entdef.userData){
				userdata[pair.first] = pair.second;
			}

			for(auto& pair: ndef.userData){
				userdata[pair.first] = pair.second;
			}

			// Set layer
			auto layerStr = findin(userdata, std::string{"anom_layer"}, std::string{"0"});
			if(layerStr.size() == 0){
				throw entdef.name + " is missing layer property";
			}
			s32 layer = std::stol(layerStr);
			assert(layer < 10);

			ent->SetLayer(layer);

			// Set up colliders
			ColliderComponent* collider = nullptr;
			if(entdef.physicsType != PhysicsType::None){
				bool dynamic = entdef.physicsType == PhysicsType::Dynamic;

				switch(entdef.colliderType){
					case ColliderType::Box:
						collider = ent->AddComponent<BoxColliderComponent>(entdef.bounds, dynamic);
						break;
					case ColliderType::Sphere:
						collider = ent->AddComponent<SphereColliderComponent>(entdef.bounds, dynamic);
						break;
					case ColliderType::Capsule:
						collider = ent->AddComponent<CapsuleColliderComponent>(entdef.bounds, dynamic);
						break;
					case ColliderType::Cone:
						collider = ent->AddComponent<ConeColliderComponent>(entdef.bounds, dynamic);
						break;
					case ColliderType::Cylinder:
						collider = ent->AddComponent<CylinderColliderComponent>(entdef.bounds, dynamic);
						break;
					case ColliderType::Mesh:
						collider = ent->AddComponent<MeshColliderComponent>(entdef.bounds, dynamic);
						break;

					default:
						throw "Collider type not implemented";
				}

				collider->collisionGroups = 1u<<layer;
			}

			auto otype = std::stol(findin(userdata, std::string{"anom_objecttype"}, std::string{"0"}));
			switch(otype){
				case 0/*World*/: break;
				case 1/*Portal*/:{
					auto dstlayerStr = findin(userdata, std::string{"anom_portaldst"}, std::string{"1"});
					s32 dstlayer = std::stol(dstlayerStr);
					assert(dstlayer < 10);

					// app->portalManager->AddPortal(ogreent, layer, dstlayer);
					ent->AddComponent<Portal>(layer, dstlayer);
					if(collider) {
						// Set as trigger
						collider->SetTrigger(true);

						// Enable interactions in destination layer
						collider->collisionGroups |= 1u<<dstlayer;
					}
					break;
				}

				case 2/*Mirror*/:{
					// TODO: Is mirror
					break;
				}
				case 3/*Interactable*/: {
					auto targetEntStr = findin(userdata, std::string{"anom_targetentity"});
					auto action = findin(userdata, std::string{"anom_interactaction"});

					if(targetEntStr.size() > 0)
						ent->AddComponent<Interactable>(targetEntStr, action);
					else
						ent->AddComponent<Interactable>(action);

					break;
				}
				case 4/*Door*/: {
					ent->AddComponent<DoorComponent>();
					break;
				}

				default: throw "Unknown object type";
			}

			// Set user data
			auto& uob = ogreent->getUserObjectBindings();
			uob.setUserAny(Ogre::Any{ent});
		}

		for(auto& child: ndef.nodes){
			nodeQueue.push({node, ent, &child});
		}
	}
}

auto BlenderSceneLoader::ParseNodes(xml_node<>* node) -> std::vector<Node> {
	std::vector<Node> nodes;
	if(!node) return nodes;

	for(auto n = node->first_node("node"); n; n = n->next_sibling("node")) {
		auto nameattr = n->first_attribute("name");
		auto posnode = n->first_node("position");
		auto rotnode = n->first_node("rotation");
		auto scalenode = n->first_node("scale");
		auto entnode = n->first_node("entity");
		auto udnode = n->first_node("user_data");

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

auto BlenderSceneLoader::ParseEntity(xml_node<>* node) -> std::shared_ptr<EntityDef> {
	auto nameattr = node->first_attribute("name");
	auto meshattr = node->first_attribute("meshFile");
	auto phystypattr = node->first_attribute("physics_type");
	auto coltypeattr = node->first_attribute("collisionPrim");
	auto dimnode = node->first_node("dimensions");

	if(!nameattr || !meshattr || !phystypattr || !dimnode) {
		error("Malformed entity");
		return nullptr;
	}

	auto e = std::make_shared<EntityDef>();
	e->name = nameattr->value();
	e->mesh = meshattr->value();
	e->bounds = ParseVec(dimnode);
	std::string phystype{phystypattr->value()};

	if(phystype == "NO_COLLISION"){
		e->physicsType = PhysicsType::None;
	}else if(phystype == "STATIC"){
		e->physicsType = PhysicsType::Static;
	}else if(phystype == "RIGID_BODY"){
		e->physicsType = PhysicsType::Dynamic;
	}else{
		error("Unsupported physics type in scene file: " + phystype);
	}

	if(e->physicsType != PhysicsType::None){
		if(!coltypeattr) {
			error("RigidBody missing collider type");
		}

		std::string coltype{coltypeattr->value()};

		if(coltype == "box") {
			e->colliderType = ColliderType::Box;
		}else if(coltype == "capsule") {
			e->colliderType = ColliderType::Capsule;
		}else if(coltype == "sphere") {
			e->colliderType = ColliderType::Sphere;
		}else if(coltype == "cylinder") {
			e->colliderType = ColliderType::Cylinder;
		}else if(coltype == "cone") {
			e->colliderType = ColliderType::Cone;
		}else if(coltype == "convex_hull") {
			e->colliderType = ColliderType::ConvexHull;
		}else if(coltype == "triangle_mesh") {
			e->colliderType = ColliderType::Mesh;
		}else {
			error("Invalid collider type in scene file: " + coltype);
		}
	}

	return e;
}

vec3 BlenderSceneLoader::ParseVec(xml_node<>* node){
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

quat BlenderSceneLoader::ParseQuaternion(xml_node<>* node){
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

auto BlenderSceneLoader::ParseUserData(xml_node<>* node) -> UserData {
	UserData ud;

	for(auto n = node; n; n = n->next_sibling("user_data")){
		auto name = n->first_attribute("name");
		auto value = n->first_attribute("value");

		if(!name || !value){
			error("Malformed userdata");
			continue;
		}

		ud[name->value()] = value->value();
	}

	return ud;
}
