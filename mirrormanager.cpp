#include <OGRE/OgreEntity.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSceneManager.h>

#include "layerrenderingmanager.h"
#include "mirrormanager.h"
#include "component.h"
#include "entity.h"
#include "app.h"

#include <algorithm>
#include <cassert>

#define PRINT(msg) std::cout << "MirrorMan: " << msg << std::endl;
//#define ERROR(msg) std::err << "MirrorMan EXCEPTION: " << msg << std::endl;

Mirror::Mirror(s32 l0, s32 l1) : Component(this) {
	layer[0] = l0;
	layer[1] = l1;

	PRINT("new mirror with id:" << id << ", layer: " << layer[0] << ", dest. layer: " << layer[1]);
}

void Mirror::OnInit() {
	auto mirrorMan = App::GetSingleton()->mirrorManager;

	//assert(layer[0] < 10 && layer[1] < 10);
	entity->ogreEntity->setRenderQueueGroup(RENDER_QUEUE_MIRRORED + mirrorId);

	mirrorMan->AddMirror(this);
}

void Mirror::OnUpdate() {
	bounceCount = 0;
}

template<>
MirrorManager* Singleton<MirrorManager>::instance = nullptr;

MirrorManager::MirrorManager() {
	// TODO: Remove if unused
}

MirrorManager::~MirrorManager() {
	// TODO: Remove if unused
}

void MirrorManager::AddMirror(Mirror* mirror) {
	if (mirror != nullptr) {
		mirror->mirrorId = (s32)mirrors.size();
		assert(mirror->mirrorId < 30);

		mirrors.push_back(mirror);

		auto& numLayers = App::GetSingleton()->layerRenderingManager->numLayers;
		numLayers = std::max((u32)std::max(mirror->layer[0], mirror->layer[1]) + 1, numLayers);
	}
}