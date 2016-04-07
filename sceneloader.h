#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "common.h"

// TEMP INTERFACE obviously
struct RawMeshData {
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

struct MaterialData {
	u8 nameLength;
	char name[256];
	vec3 color;
};

struct SceneData {
	u16 numMeshes = 0;
	RawMeshData* meshes = nullptr;

	u8  numMaterials = 0;
	MaterialData* materials = nullptr;

	u16 numEntities = 0;
	u16 numScripts = 0;
};

// TEMP INTERFACE obviously
SceneData LoadSceneData(const char* fname);
void FreeSceneData(SceneData*);

#endif