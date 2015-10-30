#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>

#include "layerrenderingmanager.h"
#include "mirrormanager.h"
#include "component.h"
#include "meshinfo.h"
#include "entity.h"
#include "app.h"
#include "camera.h"

#include <algorithm>
#include <cassert>

// TODO: Fix culling
// TODO: Lerp colors in HSV color space

#define PRINT(msg) std::cout << "MirrorMan: " << msg << std::endl;
//#define ERROR(msg) std::err << "MirrorMan EXCEPTION: " << msg << std::endl;

Mirror::Mirror(s32 l) : Component(this) {
	layer = l;
	PRINT("instantiated mirror with id:" << id << ", layer: " << layer);
}

void Mirror::OnInit() {
	auto mirrorMan = App::GetSingleton()->mirrorManager;

	assert(layer < 10);
	mirrorMan->AddMirror(this);

	entity->ogreEntity->setRenderQueueGroup(RENDER_QUEUE_MIRRORED + mirrorId);
	CalcReflectionMatrixAndClipPlane();

	// TODO: add convenience functions for ONLY setting alpha
	// SetColor(Ogre::ColourValue{0.5f, 0.8f, 1.0f, 0.5f});
	// SetColor(Ogre::ColourValue{0.398094f, 0.51999f, 0.47197f, 0.5f});
	SetColor(Ogre::ColourValue{0.49761f, 0.64999f, 0.58996f, 0.2f});
}

void Mirror::SetColor(const Ogre::ColourValue& color) {
	auto mat = entity->ogreEntity->getSubEntity(0)->getMaterial();

	mat->setSelfIllumination(color);
	mat->setDiffuse(color);
	mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
}

// Linearly interpolate between two colours
void Mirror::LerpColor(const Ogre::ColourValue& lhs, const Ogre::ColourValue& rhs, f32 delta) {
	f32 r = lhs.r + delta * (rhs.r - lhs.r);
	f32 g = lhs.g + delta * (rhs.g - lhs.g);
	f32 b = lhs.b + delta * (rhs.b - lhs.b);
	f32 a = lhs.a + delta * (rhs.a - lhs.a);

	Ogre::ColourValue rgba(r, g, b, a);
	SetColor(rgba);
}

Ogre::SubMesh* Mirror::GetSubMesh() {
	return *entity->ogreEntity->getMesh()->getSubMeshIterator().begin();
}

void Mirror::CalcReflectionMatrixAndClipPlane() {
	auto mesh = GetOgreSubMeshVertices(GetSubMesh());

	if (mesh.size() >= 3) {
		// Calculate normal from mesh vertices
		auto p1 = mesh[1] - mesh[0];
		auto p2 = mesh[2] - mesh[0];

		auto normal = p1.crossProduct(p2);
		normal.normalise();
		
		// p is a point on the plane
		auto p = vec3::ZERO;

		// Determine that point from the average of all points for later on when aligning to orientation
		for (u32 v = 0; v < mesh.size(); ++v) {
			p += mesh[v];
		}

		p /= (f32)mesh.size();

		// Offset p by the node position
		auto mirrorNode = entity->ogreEntity->getParentSceneNode();
		p += mirrorNode->_getDerivedPosition();

		// n is the normal or unit vector perpendicular to the plane
		auto n = entity->ogreEntity->getParentSceneNode()->_getDerivedOrientation() * normal;
		auto pn = p.dotProduct(n);

		// Translate and rotate to origin
		// Scale by -1 along the Y axis (up)
		// Translate and rotate back
		reflectionMat = mat4(
			1 - 2 * (n.x * n.x),	-2 * n.x * n.y,			-2 * n.x * n.z,			2 * pn * n.x,
			-2 * n.x * n.y,			1 - 2 * (n.y * n.y),	-2 * n.y * n.z,			2 * pn * n.y,
			-2 * n.x * n.z,			-2 * n.y * n.z,			1 - 2 * (n.z * n.z),	2 * pn * n.z,
			0,						0,						0,						1
		);

		// pn is length
		clipPlane = Ogre::Plane(n, pn);
	}
}

template<>
MirrorManager* Singleton<MirrorManager>::instance = nullptr;

void MakeOrtho2D(float left, float right,
				float bottom, float top,
				float zNear, float zFar,
				Ogre::Matrix4& result) {
  float width = right-left;
  float height = top-bottom;
  float depth = zFar-zNear;

  // naive initialization
  result = Ogre::Matrix4::ZERO;


}

void MirrorManager::AddMirror(Mirror* mirror) {
	if (mirror != nullptr) {
		mirror->mirrorId = (s32)mirrors.size();
		assert(mirror->mirrorId < 30);

		mirrors.push_back(mirror);

		auto& numLayers = App::GetSingleton()->layerRenderingManager->numLayers;
		numLayers = std::max((u32)mirror->layer + 1, numLayers);
	}
}

void MirrorManager::CalcCullFrustumFromBounds(Ogre::AxisAlignedBox vf) {
	vec3 size = vf.getSize();
	vec3 min = vf.getMinimum();
	vec3 max = vf.getMaximum();

	mat4 mat(
		2.0f / size.x,	0,				0,				-(max.x + min.x) / size.x,
		0,				2.0f / size.y,	0,				-(max.y + min.y) / size.y,
		0,				0,				-2.0f / size.z,	-(max.z + min.z) / size.z,
		0,				0,				0,				1.0f
	);

	cullFrustum.setCustomProjectionMatrix(true, mat);
}

void MirrorManager::UpdateCullFrustum(Ogre::Frustum* cf) {
	Ogre::AxisAlignedBox virtualFrustums;

	// TODO:
	// Calculate bounds to contain cameraFrustum and all visible virtual mirror frustums.
	// Calculate visibility of virtual mirror frustums by checking if entity is visible in camera frustum

	virtualFrustums.setMinimum(vec3(-9999, -9999, -9999)); // left, down, backward
	virtualFrustums.setMaximum(vec3(9999, 9999, 9999)); // right, up, forward

	CalcCullFrustumFromBounds(virtualFrustums);
}