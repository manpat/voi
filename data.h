#ifndef DATA_H
#define DATA_H

#include "common.h"

struct Mesh {
	enum { MaxInlineSubmeshes = 4 };

	u32 numTriangles = 0;
	u32 elementType = 0; // GL_UNSIGNED_*
	u32 vbo = 0;
	u32 ebo = 0;
	u8 elementSize = 0; // 1,2,4

	struct Submesh {
		u32 startIndex;
		u8 materialID;
		// 3B free here
	};

	u8 numSubmeshes = 0;
	union {
		// Save allocations for meshes with few materials
		Submesh submeshesInline[MaxInlineSubmeshes];
		Submesh* submeshes;
	};
};

struct Material {
	char* name;
	vec3 color;
	u32 flags = 0;
	u32 shaderID = 0;
};

struct Entity {
	enum : u8 {
		TypeGeometry,
		TypePortal,
		TypeMirror,
	};

	enum : u8 {
		FlagHidden = 1<<0,
	};

	u16 id;
	u32 flags;

	u8 layer;
	vec3 position;
	quat rotation;

	u16 parentID;
	u16 meshID;

	u8 nameLength;
	char* name;

	// u16 scriptID; // TODO
	u8 entityType; // Type*
	u8 colliderType;

	// TODO: collision stuff

	union {
		// Portal info
		struct {
			u8 targetLayer;
		} portalInfo;
	};
};

struct ShaderProgram {
	u32 program;
};

struct Scene {
	u16 numMeshes = 0;
	u16 numEntities = 0;

	u32 nameArenaSize = 0;
	char* nameArena = nullptr;
	char* nameArenaFree = nullptr;

	Entity* entities;
	Mesh* meshes;
	Material materials[256];
	ShaderProgram shaders[256]; // 0 is default shader
};

#endif