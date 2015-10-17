#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"
#include "singleton.h"

namespace Ogre {
	class Camera;
}

struct Mirror : Component {
	Mirror(s32);

	void OnInit() override;
	void CalculateReflectionMatrix();
	void SetColor(Ogre::ColourValue);
	Ogre::SubMesh* GetSubMesh();

	s32 mirrorId;
	s32 layer;
	bool isVisible = true;
	mat4 reflectionMat = mat4::IDENTITY;
};

struct MirrorManager : public Singleton<MirrorManager>, Ogre::RenderQueueListener {
	void AddMirror(Mirror*);
	std::vector<Mirror*> mirrors;
};

#endif // MIRROR_MANAGER_H