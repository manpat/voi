#include "entity.h"
#include "component.h"
#include "pool.h"

#include <algorithm>

FramePool* Entity::messagePool = nullptr;

void Entity::Init(){
	components = {};
	children = {};
	parent = nullptr;
	userdata = {};
	// id set by entity manager

	enabled = true;
}

void Entity::Destroy(){
	for(auto c: components){
		c->OnDestroy();
		delete c;
	}
	// TODO: Destroy children
}

void Entity::Update(){
	for(auto c: components){
		c->OnUpdate();
	}
}

void Entity::AddComponent(Component* c){
	if(!c) return;

	components.push_back(c);
	c->entity = this;
	c->enabled = true;
	c->OnAwake();
}

void Entity::RemoveComponent(Component* c){
	if(!c) return;

	auto it = std::find(components.begin(), components.end(), c);
	if(it == components.end()) return;

	// TODO: TEST THIS!!
	c->OnDestroy();

	*it = components.back();
	components.pop_back();
	delete c;
}

template<>
void Entity::SendMessage(const std::string& type){
	OpaqueType ot;

	for(auto c: components){
		c->OnMessage(type, ot);
	}
}
// SendMessage with arguments defined in entity.inl

#include "entitymanager.h"

void unittest_Entity(){
	struct ComponentA : Component {
		ComponentA(int _x) : x(_x) {}
		void OnAwake() { std::cout   << "A OnAwake\n"; }
		void OnDestroy() { std::cout << "A OnDestroy\n"; }
		void OnUpdate() { std::cout  << "A OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "A OnMessage " << type << "\t";
			auto data = ot.Get<std::tuple<int,int,int,int>>(false);

			if(!data) return;

			std::cout << std::get<0>(*data) << " ";
			std::cout << std::get<1>(*data) << " ";
			std::cout << std::get<2>(*data) << " ";
			std::cout << std::get<3>(*data) << " ";
			std::cout << x << "\n";
		}

		int x;
	};
	struct ComponentB : Component {
		ComponentB(int _x) : x(_x) {}
		void OnAwake() { std::cout   << "B OnAwake\n"; }
		void OnDestroy() { std::cout << "B OnDestroy\n"; }
		void OnUpdate() { std::cout  << "B OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "B OnMessage " << type << "\t";
			auto data = ot.Get<std::tuple<int,float,int,int>>(false /* Non fatal */);

			if(!data){
				std::cout << "Unexpected message data type " << ot.name << "\n";
				return;
			}

			std::cout << std::get<0>(*data) << " ";
			std::cout << std::get<1>(*data) << " ";
			std::cout << std::get<2>(*data) << " ";
			std::cout << std::get<3>(*data) << " ";
			std::cout << x << "\n";
		}

		float x;
	};

	EntityManager emgr {};
	auto e1 = emgr.CreateEntity();
	e1->AddComponent<ComponentA>(5);
	e1->AddComponent<ComponentA>(6);
	e1->AddComponent<ComponentB>(7);

	emgr.Update();
	e1->SendMessage("blah", 1, 2, 3, 4);
	e1->SendMessage("blah", 1);
	e1->SendMessage("blah");
	emgr.Update();
}