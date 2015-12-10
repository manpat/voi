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


	// Move all the UI Objects one unit infront of the camera so they're not culled
	for (auto &o : uiObjects) {
		auto uipos = camera->GetGlobalPosition() + (camera->GetForward() * o->GetPriority());
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

s32 UiManager::GetUiWidth() const {
	return uiFixedWidth;
}

s32 UiManager::GetUiHeight() const {
	return uiFixedHeight;
}

void UiManager::SetUiSize(s32 width, s32 height) {
	uiFixedWidth = width;
	uiFixedHeight = height;
}