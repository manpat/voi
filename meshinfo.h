#ifndef MESHINFO_H
#define MESHINFO_H

#include <vector>
#include "common.h"

namespace Ogre {
	class SubMesh;
}

// TODO: Make this take scale into account
std::vector<vec3> GetOgreSubMeshVertices(Ogre::SubMesh*);

#endif