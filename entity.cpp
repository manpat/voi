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

	c->OnDestroy();

	*it = components.back();
	components.pop_back();
}

void Entity::DestroyComponent(Component* c){
	if(!c) return;
	RemoveComponent(c);
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
		ComponentA(int _x) : Component{this}, x(_x) {}
		~ComponentA() { std::cout << "A Destructor\n"; }
		void OnAwake() { std::cout   << "A OnAwake\n"; }
		void OnDestroy() { std::cout << "A OnDestroy\n"; }
		void OnUpdate() { std::cout  << "A OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "A OnMessage " << type << "\t";
			auto data = ot.Get<std::tuple<int,int,int,int>>(false);

			if(!data) {
				std::cout << "A Unexpected message data type " << ot.name << "\n";
				return;
			}

			std::cout << "A ";
			std::cout << std::get<0>(*data) << " ";
			std::cout << std::get<1>(*data) << " ";
			std::cout << std::get<2>(*data) << " ";
			std::cout << std::get<3>(*data) << " ";
			std::cout << x << "\n";
		}

		int x;
	};
	struct ComponentB : Component {
		ComponentB(int _x) : Component{this}, x(_x) {}
		void OnAwake() { std::cout   << "B OnAwake\n"; }
		void OnDestroy() { std::cout << "B OnDestroy\n"; }
		void OnUpdate() { std::cout  << "B OnUpdate\n"; }
		void OnMessage(const std::string& type, const OpaqueType& ot) {
			std::cout << "B OnMessage " << type << "\t";
			auto data = ot.Get<float>(false /* Non fatal */);

			if(!data){
				std::cout << "B Unexpected message data type " << ot.name << "\n";
				return;
			}

			std::cout << "B " << *data << " ";
			std::cout << x << "\n";
		}

		float x;
	};

	EntityManager emgr {};
	auto e = emgr.CreateEntity();
	auto c1 = e->AddComponent<ComponentA>(5);
	auto c2 = e->AddComponent<ComponentA>(6);
	auto c3 = e->AddComponent<ComponentB>(7);

	std::cout << std::boolalpha;
	std::cout << "c1 Is ComponentA? " << c1->IsType<ComponentA>() << "\n";
	std::cout << "c1 Is ComponentB? " << c1->IsType<ComponentB>() << "\n";
	std::cout << "c3 Is ComponentA? " << c3->IsType<ComponentA>() << "\n";
	std::cout << "c3 Is ComponentB? " << c3->IsType<ComponentB>() << "\n";

	std::cout << "c1 SameType as c2? " << c1->SameType(c2) << "\n";
	std::cout << "c1 SameType as c3? " << c1->SameType(c3) << "\n";

	std::cout << "\n";
	std::cout << "e->FindComponent<ComponentA> == c1? " << (e->FindComponent<ComponentA>() == (Component*)c1) << "\n";
	std::cout << "e->FindComponent<ComponentB> != c2? " << (e->FindComponent<ComponentB>() != (Component*)c2) << "\n";
	std::cout << "e->FindComponent<ComponentB> == c3? " << (e->FindComponent<ComponentB>() == (Component*)c3) << "\n";

	std::cout << "\n";
	emgr.Update();
	e->SendMessage("four", 1, 2, 3, 4);
	e->SendMessage("one ", 1.f);
	e->SendMessage("null");
	e->SendMessage("comp", c1);
	emgr.Update();
	e->RemoveComponent(c1);
	e->RemoveComponent(c2);
	emgr.Update();
}