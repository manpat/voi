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
	char name[256];
	vec3 color;
	u32 flags = 0;
	u32 shaderID = 0;
};

struct Entity {};
struct Scene {
	u16 numMeshes = 0;

	Mesh* meshes;
	Material materials[256];
};

#endif