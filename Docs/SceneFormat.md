Voi scene files (.voi) are a packed binary format
All data is little endian.
ALL IDs start at 1. ID of zero should be treated as invalid/null

Unless there's a good reason to do otherwise, all information and data related to a level
should be packed into the scene file.

This includes:
	Entities
		Unique name
		Predetermined id?
			- this would mean that you could resolve references to entities at export
		Transforms
		Optional mesh id
		Heirarchy information
		Any info about behaviours
		Physics info

	'Regions' - pretty much just triggers, could be entities
		Audio/Fog regions
		Half-life points

	Meshes
		Mesh id
		Single static mesh for level geometry in a layer
		Material info

	Scripts

	Any scene specific stuff
		Fog settings (although this could be controlled with regions)
		Music/audio
		Sky/clear color (this could be specific to layers also)
		Player spawn point


Format
======

Scene {
	"VOI"
	u8 					versionNumber (should be 1)
	
	u32					numMeshes
	Mesh[numMeshes]		meshes

	u32					numEntities
	Entity[numEntities]	entities
}

Mesh {
	"MESH"
	u32						chunkSize
	u32						id
	u32						numVertices
	f32[numVertices * 3] 	vertices
	u32						numTriangles

	if(numVertices < 256) {
		u8 [numTriangles*3]	triangles
	}else if(numVertices < 65536) {
		u16[numTriangles*3]	triangles
	}else if(numVertices < INT_MAX) {
		u32[numTriangles*3]	triangles
	}

	// Material info
}

Entity {
	"ENTY"
	u32				chunkSize
	u32				id
	u32				meshID (optional)
}