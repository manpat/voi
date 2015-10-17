#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"
#include "singleton.h"

namespace Ogre {
	class Camera;
}

struct Mirror : Component {
	Mirror(s32 l0, s32 l1);

	void OnInit() override;
	void OnUpdate() override;

	void CalculateReflectionMatrix();
	void SetColor(Ogre::ColourValue color);
	Ogre::SubMesh* GetSubMesh();

	s32 mirrorId;
	s32 layer[2]; // Layers that should be reflected (layer of mirror, normally hidden layer)
	bool isVisible = true;

	mat4 reflectionMat = mat4::IDENTITY;
};

struct MirrorManager : public Singleton<MirrorManager>, Ogre::RenderQueueListener {
public:
	MirrorManager();
	~MirrorManager();

	void AddMirror(Mirror*);

	std::vector<Mirror*> mirrors;
};

#endif // MIRROR_MANAGER_H