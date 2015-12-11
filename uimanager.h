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
	T* CreateObject(const std::string& name, const std::string& parent = "") {
		T* o = new T{};

		if (parent.empty())	{
			o->node = App::GetSingleton()->rootNode->createChildSceneNode(name);
		}
		else {
			auto p = (Ogre::SceneNode*)App::GetSingleton()->rootNode->getChild(parent);
			o->node = p->createChildSceneNode(name);
		}

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

	void DestroyAllObjects();

	s32 GetUiWidth() const;
	s32 GetUiHeight() const;

	void SetUiSize(s32 w, s32 h);

private:
	s32 uiFixedWidth = 1920;
	s32 uiFixedHeight = 1080;
};

#endif//UNIMANAGER_H