Meshes
======
Meshes will need to be split into several 'submeshes', one for each material used.
Submeshes can just be indices into a single element buffer since faces
should be sorted based on material anyway to reduce draw calls.
This can be avoided if vertices are duplicated such that you can just assign them a color.
This would mean much more memory usage but could potentially be faster.

// An immutable mesh
struct Mesh {
	u32 numElements
	u32 elementType // GL_UNSIGNED_*
	u32 vbo
	u32 ebo

	struct Submesh {
		u32 startIndex
		u8 materialID
		// 3B free here
	}

	u8 numSubmeshes
	union {
		// Save allocations for meshes with few materials
		Submesh[4] submeshesInline
		Submesh* submeshes
	}
}

Materials
=========
At the moment, there is an assumption that there will never be more than 255 materials in a scene.
I think this is reasonable but if it becomes restrictive later on down the line it can be changed.

struct Material {
	char[256] name
	f32[3] color
	u32 flags
	u32 shaderID // zero for default
}


Entities
========
Entities essentially represent all thing that can have a position.
Possible entity types include:
- Static geometry
- Triggers / Volumes / Regions
	- Fog/Audio regions
	- Halflife points
- Interactive/scripted objects
	- Bells
	- Doors
	- Buttons
- The player
- Portals / mirrors
- Particle systems (NOT individual particles obvs)

struct Entity {
	u16 id
	u8 entityType
	char[256] name
	u32 flags

	vec3 position
	quat rotation

	u16 parentID
	u16 meshID

	u16 scriptID // Unsure how this will be handled

	// Collider data
	// Specific entity data
}