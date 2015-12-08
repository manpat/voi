#ifndef MESHINFO_H
#define MESHINFO_H

#include <vector>
#include "common.h"

namespace Ogre {
	class Mesh;
	class SubMesh;
}

// TODO: Make these take scale into account

// GetOgreSubMeshVertices gets the vertices referenced by a submesh
std::vector<vec3> GetOgreSubMeshVertices(Ogre::SubMesh*);

// GetOgreSubMeshVerticesFlat is similar to GetOgreSubMeshVertices except
//	it duplicates vertices to form triangles
std::vector<vec3> GetOgreSubMeshVerticesFlat(Ogre::SubMesh*);

#endif