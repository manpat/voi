#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSubMesh.h>

#include "layerrenderingmanager.h"
#include "mirrormanager.h"
#include "component.h"
#include "entity.h"
#include "app.h"
#include "meshinfo.h"

#include <algorithm>
#include <cassert>

// TODO: Clipping issues
// TODO: Multiple mirror support
// TODO: Test with portals
// TODO: Color lerping

#define PRINT(msg) std::cout << "MirrorMan: " << msg << std::endl;
//#define ERROR(msg) std::err << "MirrorMan EXCEPTION: " << msg << std::endl;

Mirror::Mirror(s32 l) : Component(this) {
	layer = l;
	PRINT("new mirror with id:" << id << ", layer: " << layer);
}

void Mirror::OnInit() {
	auto mirrorMan = App::GetSingleton()->mirrorManager;

	assert(layer < 10);
	mirrorMan->AddMirror(this);

	entity->ogreEntity->setRenderQueueGroup(RENDER_QUEUE_MIRRORED + mirrorId);
	CalculateReflectionMatrix();
}

void Mirror::SetColor(Ogre::ColourValue color) {
	entity->ogreEntity->getSubEntity(0)->getMaterial()->setSelfIllumination(color);
}

Ogre::SubMesh* Mirror::GetSubMesh() {
	return *entity->ogreEntity->getMesh()->getSubMeshIterator().begin();
}

void Mirror::CalculateReflectionMatrix() {
	auto mesh = GetOgreSubMeshVertices(GetSubMesh());

	if (mesh.size() >= 3) {
		// Calculate normal from mesh vertices
		auto p1 = mesh[1] - mesh[0];
		auto p2 = mesh[2] - mesh[0];

		auto normal = p1.crossProduct(p2);
		normal.normalise();

		auto mirrorNode = entity->ogreEntity->getParentSceneNode();

		// p is a point on the plane and n is the normal or unit vector perpecdicular to the plane
		auto n = mirrorNode->_getDerivedOrientation() * normal;
		auto p = mirrorNode->_getDerivedPosition();
		auto pn = p.dotProduct(n);

		// Translate and rotate to origin
		// Scale by -1 along the XZ axes (Y up)
		// Translate and rotate back
		reflectionMat = mat4(
			1 - 2 * (n.x * n.x),	-2 * n.x * n.y,			-2 * n.x * n.z,			2 * pn * n.x,
			-2 * n.x * n.y,			1 - 2 * (n.y * n.y),	-2 * n.y * n.z,			2 * pn * n.y,
			-2 * n.x * n.z,			-2 * n.y * n.z,			1 - 2 * (n.z * n.z),	2 * pn * n.z,
			0,						0,						0,						1
		);
	}
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