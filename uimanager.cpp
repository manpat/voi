#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>

#include "uimanager.h"
#include "uiobject.h"
#include "uiimage.h"
#include "apptime.h"
#include "camera.h"
#include "entity.h"
#include "uitext.h"
#include "app.h"

template<>
UiManager* Singleton<UiManager>::instance = nullptr;

UiManager::UiManager() {
	instance = this;
}

UiManager::~UiManager() {
	DestroyAllObjects();
	instance = nullptr;
}

void UiManager::Update() {
	auto camera = App::GetSingleton()->camera->entity;
	auto uipos = camera->GetGlobalPosition() + camera->GetForward();

	// Move all the UI Objects one unit infront of the camera so they're not culled
	for (auto &o : uiObjects) {
		o->node->_setDerivedPosition(uipos);
	}
}

void UiManager::DestroyAllObjects() {
	for (auto it = uiObjects.begin(); it != uiObjects.end();) {
		auto o = *it;

		if (!o) {
			++it;
			continue;
		}

		it = uiObjects.erase(it);

		delete o;
		o = nullptr;
	}

	assert(uiObjects.empty());
}