#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <vector>

#include "app.h"
#include "singleton.h"

#include <OGRE/OgreSceneNode.h>

struct UiObject;

struct UiManager : Singleton<UiManager> {
	std::vector<UiObject*> uiObjects;

	UiManager();
	~UiManager();

	void Update();

	template<typename T>
	T* CreateObject(const std::string& name) {
		T* o = new T{};

		o->node = App::GetSingleton()->rootNode->createChildSceneNode(name);
		o->Init();

		uiObjects.push_back(o);

		return o;
	}

	template<typename T>
	T* FindObject(const std::string& name) {
		auto o = std::find_if(uiObjects.begin(), uiObjects.end(), [&name](const T* obj) {
			return name == obj->GetName();
		});

		if (o == uiObjects.end()) return nullptr;
		return *o;
	}
	
	template<typename T>
	void DestroyObject(T* o) {
		if (!o) return;

		auto end = uiObjects.end();
		uiObjects.erase(std::remove(uiObjects.begin(), end, o), end);

		delete o;
	}

};

#endif//UNIMANAGER_H