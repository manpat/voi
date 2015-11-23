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
	//uiObjects[0]->SetSize(abs((sin(AppTime::appTime) * 128)), abs(sin(AppTime::appTime + PI / 2) * 128));
	//uiObjects[0]->SetPosition((sin(AppTime::appTime)), sin(AppTime::appTime + PI / 2));
}