#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "common.h"
#include "data.h"
#include "voigl.h"

struct MeshData {
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

struct EntityData {
	u8 nameLength;
	char name[256];
	vec3 position;
	vec3 rotation;
	vec3 scale;
	u32 layers;

	u32 flags;
	u16 parentID;
	u16 meshID;
	u8 entityType;
	u8 colliderType;

	u8 updateCallbackLen;
	char updateCallback[256];

	// TODO: MORE
	
	static constexpr u32 MaxEntitySpecificData = 1024;
	u8 entitySpecificData[MaxEntitySpecificData];
};

struct SceneData {
	u16 numMeshes = 0;
	MeshData* meshes = nullptr;

	u8  numMaterials = 0;
	vec3* materials = nullptr;

	u16 numEntities = 0;
	EntityData* entities = nullptr;
};

SceneData LoadSceneData(const char* fname);
void FreeSceneData(SceneData*);

#if 0
#define SCENEPRINT(...) Log(__VA_ARGS__)
#else
#define SCENEPRINT(...) [](...){}(__VA_ARGS__) // Get rid of warnings
#endif

#endif