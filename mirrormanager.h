#ifndef MIRROR_MANAGER_H
#define MIRROR_MANAGER_H

#include <OGRE/OgreRenderQueueListener.h>
#include <OGRE/OgreFrustum.h>

//#include "common.h"
#include "component.h"
#include "singleton.h"

class InfiniteFrustum : public Ogre::Frustum
{
public:
	/*InfiniteFrustum() : Frustum() {
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_LEFT].normal = vec3::NEGATIVE_UNIT_X;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_LEFT].d = 9999999999999999999.0f;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_RIGHT].normal = vec3::UNIT_X;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_RIGHT].d = 9999999999999999999.0f;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_TOP].normal = vec3::NEGATIVE_UNIT_Y;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_TOP].d = 9999999999999999999.0f;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_BOTTOM].normal = vec3::UNIT_Y;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_BOTTOM].d = 9999999999999999999.0f;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_NEAR].normal = vec3::NEGATIVE_UNIT_Z;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_NEAR].d = 9999999999999999999.0f;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_FAR].normal = vec3::UNIT_Z;
		mFrustumPlanes[Ogre::FRUSTUM_PLANE_FAR].d = 9999999999999999999.0f;
	}*/

	//virtual bool isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy = 0) const {return true;};
	//virtual bool isVisible(const Sphere& bound, FrustumPlane* culledBy = 0) const {return true;};
	//virtual bool isVisible(const Vector3& vert, FrustumPlane* culledBy = 0) const {return true;};
	//bool projectSphere(const Sphere& sphere, 
	//	Real* left, Real* top, Real* right, Real* bottom) const {*left = *bottom = -1.0f; *right = *top = 1.0f; return true;};
	//Real getNearClipDistance(void) const {return 1.0;};
	//Real getFarClipDistance(void) const {return 9999999999999.0f;};
	//const Plane& getFrustumPlane( unsigned short plane ) const
	//{
	//	return mFrustumPlanes[plane];
	//}
};

struct Mirror : Component {
	Mirror(s32);

	void OnInit() override;
	void CalcReflectionMatrixAndClipPlane();
	void SetColor(const Ogre::ColourValue&);
	void LerpColor(const Ogre::ColourValue&, const Ogre::ColourValue&, f32);
	Ogre::SubMesh* GetSubMesh();

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
	void UpdateCullFrustum(Ogre::Frustum*);

	std::vector<Mirror*> mirrors;
	//InfiniteFrustum cullFrustum;
	Ogre::Frustum cullFrustum;
};

#endif // MIRROR_MANAGER_H