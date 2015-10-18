#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>

#include "component.h"
#include "singleton.h"

struct Mirror : Component {
	Mirror(s32);

	void OnInit() override;
	void CalculateReflectionMatrixAndClipPlane();
	void SetColor(Ogre::ColourValue);
	Ogre::SubMesh* GetSubMesh();

	s32 mirrorId;
	s32 layer;
	bool isVisible = true;
	mat4 reflectionMat = mat4::IDENTITY;
	Ogre::Plane clipPlane;
};

struct MirrorManager : public Singleton<MirrorManager>, Ogre::RenderQueueListener {
	void AddMirror(Mirror*);
	std::vector<Mirror*> mirrors;
};

#endif // MIRROR_MANAGER_H