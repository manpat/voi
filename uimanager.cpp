#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>

#include "uimanager.h"
#include "uiobject.h"
#include "uiimage.h"
#include "apptime.h"
#include "uitext.h"
#include "app.h"

template<>
UiManager* Singleton<UiManager>::instance = nullptr;

UiManager::UiManager() {
}

UiManager::~UiManager() {
}

void UiManager::Update() {

}