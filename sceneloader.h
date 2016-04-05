#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "common.h"

// TEMP INTERFACE obviously
struct Mesh {
	u32 numVertices;
	vec3* vertices;

	u32 numTriangles;
	union {
		u8*  triangles8;
		u16* triangles16;
		u32* triangles32;
	};
	u8* materialIDs;
};

struct Material {
	u8 nameLength;
	char* name;
	vec3 color;
};

struct Scene {
	u16 numMeshes = 0;
	Mesh* meshes = nullptr;

	u8  numMaterials = 0;
	u16 numEntities = 0;
	u16 numScripts = 0;
};

// TEMP INTERFACE obviously
Scene LoadScene(const char* fname);

#endif