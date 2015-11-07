#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreColourValue.h>

#include "layerrenderingmanager.h"
#include "mirrormanager.h"
#include "component.h"
#include "entity.h"
#include "app.h"
#include "camera.h"

#include <algorithm>
#include <cassert>

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

	// Default mirror colour
	if (GetColour() == Ogre::ColourValue::Black) {
		SetColour(Ogre::ColourValue(0.49761f, 0.64999f, 0.58996f, 0.2f));
	} else {
		SetOpacity(0.2f);
	}
}

// Apply new colour to mirror
void Mirror::SetColour(const Ogre::ColourValue& color) {
	auto mat = entity->ogreEntity->getSubEntity(0)->getMaterial();

	mat->setDiffuse(color);
	mat->setSelfIllumination(color);
	mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
}

// Retreive colour of mirror
Ogre::ColourValue Mirror::GetColour() {
	return entity->ogreEntity->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getDiffuse();
}

// Set opacity or alpha component only of mirror colour
void Mirror::SetOpacity(const f32 a) {
	auto colour = GetColour();
	SetColour(Ogre::ColourValue(colour.r, colour.g, colour.b, a));
}

// Linearly interpolate between two colours and apply to mirror
void Mirror::LerpColor(const Ogre::ColourValue& lhs, const Ogre::ColourValue& rhs, const f32 delta) {
	Ogre::ColourValue lhsv, rhsv;

	// Convert to HSV colour space
	lhs.getHSB(&lhsv.r, &lhsv.g, &lhsv.b);
	rhs.getHSB(&rhsv.r, &rhsv.g, &rhsv.b);

	// Lerp between hues, saturations and values by delta
	f32 h = lhsv.r + delta * (rhsv.r - lhsv.r);
	f32 s = lhsv.g + delta * (rhsv.g - lhsv.g);
	f32 v = lhsv.b + delta * (rhsv.b - lhsv.b);

	// Convert back to RGB colour space
	Ogre::ColourValue rgba(0, 0, 0, lhs.a + delta * (rhs.a - lhs.a));
	rgba.setHSB(h, s, v);

	// Apply to mirror
	SetColour(rgba);
}

void Mirror::CalcReflectionMatrixAndClipPlane() {
	auto plane = entity->GetWorldPlaneFromMesh();

	auto n = -plane.normal;
	auto l = plane.d;

	// Translate and rotate to origin
	// Scale by -1 along the Y axis (up)
	// Translate and rotate back
	reflectionMat = mat4(
		1 - 2 * (n.x * n.x),	-2 * n.x * n.y,			-2 * n.x * n.z,			2 * l * n.x,
		-2 * n.x * n.y,			1 - 2 * (n.y * n.y),	-2 * n.y * n.z,			2 * l * n.y,
		-2 * n.x * n.z,			-2 * n.y * n.z,			1 - 2 * (n.z * n.z),	2 * l * n.z,
		0,						0,						0,						1
	);

	clipPlane = Ogre::Plane(n, l);
}

template<>
MirrorManager* Singleton<MirrorManager>::instance = nullptr;

void MirrorManager::AddMirror(Mirror* mirror) {
	if (mirror != nullptr) {
		mirror->mirrorId = (s32)mirrors.size();
		assert(mirror->mirrorId < 30);

		mirrors.push_back(mirror);

		auto& numLayers = App::GetSingleton()->layerRenderingManager->numLayers;
		numLayers = std::max((u32)mirror->layer + 1, numLayers);
	}
}

void MirrorManager::CalcCullFrustumFromBounds(Ogre::AxisAlignedBox bounds) {
	// Get extents from axis aligned box bounds
	vec3 size = bounds.getSize();
	vec3 min = bounds.getMinimum();
	vec3 max = bounds.getMaximum();

	// Build orthogonal matrix from bounds
	mat4 frustumMat(
		2.0f / size.x,	0,				0,				-(max.x + min.x) / size.x,
		0,				2.0f / size.y,	0,				-(max.y + min.y) / size.y,
		0,				0,				2.0f / size.z,	-(max.z + min.z) / size.z,
		0,				0,				0,				1.0f
	);

	// Pass in matrix to get orthogonal culling frustum
	cullFrustum.setCustomProjectionMatrix(true, frustumMat);
}

void MirrorManager::UpdateCullFrustum(Ogre::Camera* camFrustum) {
	Ogre::AxisAlignedBox camBounds;
	vec4 camFrustumCorners[] = {
		// Near
		{-1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, -1, 1, 1},
		{-1, -1, 1, 1},
		// Far
		{-1, 1, -1, 1},
		{1, 1, -1, 1},
		{1, -1, -1, 1},
		{-1, -1, -1, 1}
	};

	// Get inverse view projection matrix
	auto invCamFrustumMat = (camFrustum->getProjectionMatrix() * camFrustum->getViewMatrix(true)).inverse();

	// Iterate frustum corners
	for (u8 c = 0; c < 8; ++c) {
		// Map frustum to unit cube
		camFrustumCorners[c] = invCamFrustumMat * camFrustumCorners[c];
		camFrustumCorners[c] /= camFrustumCorners[c].w;

		// Flatten vector and merge camera frustum
		camBounds.merge(vec3(camFrustumCorners[c].x, camFrustumCorners[c].y, camFrustumCorners[c].z));
	}

	// Set initial bounds to contain camera frustum
	Ogre::AxisAlignedBox bounds(camBounds);

	// Iterate mirrors
	for (auto m = mirrors.begin(); m < mirrors.end(); ++m) {
		// Skip invisible mirrors
		if (!(*m)->isVisible || !camBounds.intersects((*m)->entity->ogreEntity->getWorldBoundingBox(true))) {
			continue;
		}

		// Iterate frustum corners
		for (u8 c = 0; c < 8; ++c) {
			// Multiply flattened vector by mirrors reflection matrix and merge virtual frustum
			bounds.merge((*m)->reflectionMat * vec3(camFrustumCorners[c].x, camFrustumCorners[c].y, camFrustumCorners[c].z));
		}
	}

	CalcCullFrustumFromBounds(bounds);
}