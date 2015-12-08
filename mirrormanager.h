#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>
#include <OGRE/OgreFrustum.h>

#include "component.h"
#include "singleton.h"

struct Mirror : Component {
	Mirror(s32);

	void OnInit() override;
	void SetColour(const Ogre::ColourValue&);
	Ogre::ColourValue GetColour();
	void SetOpacity(const f32);
	void LerpColor(const Ogre::ColourValue&, const Ogre::ColourValue&, const f32);
	void CalcReflectionMatrixAndClipPlane();

	s32 mirrorId;
	s32 layer;
	bool isVisible = true;
	mat4 reflectionMat = mat4::IDENTITY;
	Ogre::Plane clipPlane;
	Ogre::Frustum cullFrustum;
};

struct MirrorManager : public Singleton<MirrorManager>, Ogre::RenderQueueListener {
	void AddMirror(Mirror*);
	void CalcCullFrustumFromBounds(Ogre::AxisAlignedBox);
	void UpdateCullFrustum(Ogre::Camera*);

	std::vector<Mirror*> mirrors;
	Ogre::Frustum cullFrustum;
};

#endif // MIRROR_MANAGER_H