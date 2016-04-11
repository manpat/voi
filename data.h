#ifndef DATA_H
#define DATA_H

#include "common.h"

struct Mesh {
	enum { MaxInlineSubmeshes = 4 };

	static constexpr u32 ElementTypeToGL[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};

	u32 numTriangles = 0;
	u32 vbo = 0;
	u32 ebo = 0;
	u8 elementType = 0; // 0,1,2

	struct Submesh {
		u32 triangleCount;
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
		TypeTrigger,

		// Specialised triggers
		TypeHalflifePoint,
		TypeAudioRegion,
		TypeFogRegion,
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
			// vec4 clipPlane;
		} portalInfo;
	};
};

struct ShaderProgram {
	u32 program;

	u32 materialColorLoc = 0;
	u32 clipPlaneLoc = 0;

	u32 viewProjectionLoc = 0;
	u32 modelLoc = 0;
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