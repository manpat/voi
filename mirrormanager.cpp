#include <OGRE/OgreEntity.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSceneManager.h>

#include "mirrormanager.h"
#include "component.h"
#include "entity.h"
#include "app.h"


#define PRINT(msg) std::cout << "MirrorMan: " << msg << std::endl;
//#define ERROR(msg) std::err << "MirrorMan EXCEPTION: " << msg << std::endl;

Mirror::Mirror(s32 l0, s32 l1) : Component(this) {
	layer[0] = l0;
	layer[1] = l1;

	PRINT("new mirror with id:" << id << ", layer: " << layer[0] << ", dest. layer: " << layer[1]);
}

template<>
MirrorManager* Singleton<MirrorManager>::instance = nullptr;

MirrorManager::MirrorManager() {
	App::GetSingleton()->sceneManager->addRenderQueueListener(this);

	PRINT("manager was employed!");
}

MirrorManager::~MirrorManager() {
	App::GetSingleton()->sceneManager->removeRenderQueueListener(this);

	PRINT("manager was fired!");
}

void MirrorManager::renderQueueStarted(u8 queueId, const std::string& invocation, bool& skipThisInvocation) {

}

void MirrorManager::renderQueueEnded(u8 queueId, const std::string& invocation, bool& repeatThisInvocation) {

}

void MirrorManager::AddMirror(Mirror* mirror) {
	mirrors.push_back(mirror);
}