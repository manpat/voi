#include "blendersceneloader.h"

#include <iostream>
#include <fstream>
#include <queue>

#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreEntity.h>

#include "halflifepointcomponent.h"
#include "layerrenderingmanager.h"
#include "physicsmanager.h"
#include "soundcomponent.h"
#include "synthcomponent.h"
#include "generictrigger.h"
#include "portalmanager.h"
#include "mirrormanager.h"
#include "entitymanager.h"
#include "doorcomponent.h"
#include "movableobject.h"
#include "interactable.h"
#include "checkpoint.h"
#include "entity.h"
#include "bell.h"
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
	environment = ParseEnvironment(node->first_node("environment"));
	delete[] data;

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Construct scene
	ConstructScene(app);
}

void BlenderSceneLoader::ConstructScene(App* app){
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
		auto n = nodeQueue.front();
		nodeQueue.pop();

		auto& ndef = *n.ndef;
		if(ndef.name == "Player") {
			assert((ndef.nodes.size() == 1) && "Player node must have single child node for orientation");
			auto& cam = ndef.nodes[0];

			app->playerSpawnPosition = ndef.position;

			quat playerSpawnPitch(cam.rotation.getPitch() - Ogre::Degree(90), vec3::UNIT_X);
			quat playerSpawnYaw(ndef.rotation.getYaw(), vec3::UNIT_Y);

			app->playerSpawnOrientation = playerSpawnPitch * playerSpawnYaw;

			continue;
		}

		auto node = n.parentNode->createChildSceneNode(ndef.name, ndef.position, ndef.rotation);
		node->setScale(ndef.scale);

		Entity* ent = nullptr;

		if(ndef.entity){
			// If entities were stored in a queue like nodes, then they could be
			//	iterated through to find portals
			auto& entdef = *ndef.entity;

			// Don't bother with entdef.name, because nothing uses it anyway
			// Entity::GetName uses the scenenode's name
			auto ogreent = app->sceneManager->createEntity(/*entdef.name, */entdef.mesh);
			node->attachObject(ogreent);

			ent = entMgr->CreateEntity(node);
			ent->ogreEntity = ogreent;
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
			auto hiddenStr = findin(userdata, std::string{"anom_hidden"}, std::string{"0"});
			auto invisibleStr = findin(userdata, std::string{"anom_invisible"}, std::string{"0"});
			auto ignorefogStr = findin(userdata, std::string{"anom_ignorefog"}, std::string{"0"});
			if(layerStr.size() == 0){
				throw entdef.name + " is missing layer property";
			}
			s32 layer = std::stol(layerStr);
			assert(layer < 10);

			ent->SetLayer(layer, hiddenStr == "1");
			ent->ogreEntity->setVisible(invisibleStr == "0");

			if(ignorefogStr != "0") {
				auto numSubEnts = ent->ogreEntity->getNumSubEntities();
				for(u32 i = 0; i < numSubEnts; i++) {
					ent->ogreEntity->getSubEntity(i)->getMaterial()->setFog(true);
				}
			}

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
					case ColliderType::ConvexHull:
						collider = ent->AddComponent<ConvexHullColliderComponent>(entdef.bounds, dynamic);
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

					ent->AddComponent<Portal>(layer, dstlayer);
					if(collider) {
						// Set as trigger
						collider->SetTrigger(true);

						// Enable interactions in destination layer
						collider->collisionGroups |= 1u<<dstlayer;
					}
				} break;

				case 2/*Mirror*/:{
					// auto waterParticles = findin(userdata, std::string{"anom_waterparticles"}, std::string{}) == "1";
					ent->AddComponent<Mirror>(layer);
				} break;

				case 3/*Interactable*/: {
					auto targetEntStr = findin(userdata, std::string{"anom_targetentity"});
					auto action = findin(userdata, std::string{"anom_interactaction"});

					if(targetEntStr.size() > 0)
						ent->AddComponent<Interactable>(targetEntStr, action);
					else
						ent->AddComponent<Interactable>(action);
				} break;

				case 4/*Door*/: {
					auto lockCount = std::stol(findin(userdata, std::string{"anom_doorcount"}, std::string{"1"}));
					auto doorTime = std::stof(findin(userdata, std::string{"anom_doormovetime"}, std::string{"2.0"}));
					auto doorOrdered = findin(userdata, std::string{"anom_doorordered"}) == "1";
					ent->AddComponent<DoorComponent>(lockCount, doorOrdered, doorTime);
				} break;

				case 5/*Generic Trigger*/:{
					collider->SetTrigger(true);
					ent->SetVisible(false);

					auto targetEntStr = findin(userdata, std::string{"anom_targetentity"});
					auto action = findin(userdata, std::string{"anom_interactaction"});

					if(targetEntStr.size() > 0)
						ent->AddComponent<GenericTrigger>(targetEntStr, action);
					else
						ent->AddComponent<GenericTrigger>(action);
				} break;

				case 6/*Halflife Point*/:{
					auto dstlevel = findin(userdata, std::string{"anom_newarea"});
					if(dstlevel.size() == 0) throw "Invalid 'newarea' for level trigger " + ent->GetName();

					ent->AddComponent<HalfLifePointComponent>(dstlevel);
					collider->SetTrigger(true);
					ent->ogreEntity->setVisible(false);
				} break;

				case 7/*Level Entry*/:{
					if(collider) collider->SetTrigger(true);
					ent->SetVisible(false);
				} break;

				case 8/*Checkpoint*/:{
					collider->SetTrigger(true);
					ent->SetVisible(false);
					ent->AddComponent<Checkpoint>();
				} break;

				case 9/*Movable Object*/:{
					ent->AddComponent<Movable>();
				} break;

				case 10/*Bells*/:{
					u32 bellNumber = 0;

					auto targetEntStr = findin(userdata, std::string{"anom_targetentity"});
					auto bellNumberStr = findin(userdata, std::string{"anom_bellnumber"});
					if(targetEntStr.size() == 0) {
						throw "Bell " + ent->GetName() + " is missing a target entity";
					}
					if(bellNumberStr.size() > 0) {
						bellNumber = std::stol(bellNumberStr);
					}

					ent->AddComponent<Bell>(targetEntStr, bellNumber);
				} break;

				default: throw "Unknown object type";
			}

			// Set user data
			auto& uob = ogreent->getUserObjectBindings();
			uob.setUserAny(Ogre::Any{ent});
		}

		// Create entity with sound/synth component if soundtype found
		s32 soundtype = std::stol(findin(ndef.userData, std::string{"anom_soundtype"}, std::string{"0"}));
		if(soundtype > 0){
			if(!ent){
				ent = entMgr->CreateEntity(node);
				ent->userdata = ndef.userData;
				if(n.parent){
					n.parent->AddChild(ent);
				}
			}

			f32 soundsize = 1.0;
			auto soundsizeStr = findin(ndef.userData, std::string{"anom_soundsize"}, std::string{"1.0"});
			if(soundsizeStr.size() > 0) soundsize = std::stof(soundsizeStr);

			switch(soundtype){
				case 1: {
					auto soundpath = findin(ndef.userData, std::string{"anom_soundpath"});
					ent->AddComponent<SoundComponent>(soundpath, soundsize);
					break;
				}

				case 2: {
					auto synthname = findin(ndef.userData, std::string{"anom_soundsynth"});
					auto hasReverbStr = findin(ndef.userData, std::string{"anom_soundhasreverb"});

					auto synth = ent->AddComponent<SynthComponent>(synthname, soundsize);

					if(hasReverbStr == "1") {
						f32 synthreverb = 10.f;
						f32 synthmix = 0.f;

						auto reverbStr = findin(ndef.userData, std::string{"anom_soundreverb"});
						auto mixStr = findin(ndef.userData, std::string{"anom_soundmix"});

						if(reverbStr.size() > 0) synthreverb = std::stof(reverbStr);
						if(mixStr.size() > 0) synthmix = std::stof(mixStr);

						synth->SetUpReverb();
						synth->SetReverbTime(synthreverb);
						synth->SetReverbMix(synthmix);
					}

					break;
				}

				default: throw "Invalid sound type for entity " + ent->GetName();
			}
		}

		for(auto& child: ndef.nodes){
			nodeQueue.push({node, ent, &child});
		}
	}
}

auto BlenderSceneLoader::ParseEnvironment(rapidxml::xml_node<>* node) -> EnvironmentDef {
	EnvironmentDef env{};
	if(!node) return env;

	auto udnode = node->first_node("user_data");
	if(!udnode) return env;

	auto ud = ParseUserData(udnode);
	auto fogtype = std::stol(findin(ud, std::string{"anom_fogtype"}, std::string{"0"}));
	assert(fogtype >= 0 && fogtype <= 3);

	env.fogType = static_cast<FogType>(fogtype);
	env.fogDensity = (f32)std::stod(findin(ud, std::string{"anom_fogdensity"}, std::string{"0.001"}));
	env.fogStart = (f32)std::stod(findin(ud, std::string{"anom_foglinearstart"}, std::string{"0"}));
	env.fogEnd = (f32)std::stod(findin(ud, std::string{"anom_foglinearstart"}, std::string{"1"}));

	auto skyStr = findin(ud, std::string{"anom_skycolor"}, std::string{"0 0 0"});
	auto ambiStr = findin(ud, std::string{"anom_ambientcolor"}, std::string{"0 0 0"});
	auto fogColStr = findin(ud, std::string{"anom_fogcolor"}, std::string{"0 0 0"});

	auto ParseColor3 = [](std::string str, f32 (&c)[3]) {
		try {
			u64 idx = 0;
			for(u8 i = 0; i < 3; i++){
				c[i] = (f32)std::stod(str, &idx);
				str = str.substr(idx);
			}

		// Ignore the actual exception
		} catch(const std::exception&) {
			c[0] = c[1] = c[2] = 1.f;
			std::cout << "Color parsing failed" << std::endl;
		}
	};

	ParseColor3(skyStr, env.skyColor);
	ParseColor3(ambiStr, env.ambientColor);
	ParseColor3(fogColStr, env.fogColor);

	return env;
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
			std::cout << "RigidBody "+ e->name +" missing collider type, defaulting to mesh collider\n";
			e->colliderType = ColliderType::Mesh;

			return e;
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
	if(!node) return ud;

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