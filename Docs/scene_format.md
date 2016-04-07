Voi scene files (.voi) are a packed binary format
All data is little endian.
ALL IDs start at 1. ID of zero should be treated as invalid/null

All chunks begin with a 4 byte stamp
The main chunk's stamp contains its version number

Unless there's a good reason to do otherwise, all information and data related to a level
should be packed into the scene file.

This includes:
- Entities
	- Unique name
	- Predetermined id?
		- this would mean that you could resolve references to entities at export
	- Transforms
	- Optional mesh id
	- Heirarchy information
	- Any info about behaviours
	- Physics info

- 'Regions' - pretty much just triggers, could be entities
	- Audio/Fog regions
	- Half-life points

- Meshes
	- Mesh id
	- Single static mesh for level geometry in a layer
	- Material info

- Scripts

- Any scene specific stuff
	- Fog settings (although this could be controlled with regions)
	- Music/audio
	- Sky/clear color (this could be specific to layers also)
	- Player spawn point (could be an entity)


Format
======
	Scene {
		"VOI"
		u8 						versionNumber (should be 1)
		// Scene name?
		// Scene info
		
		u16						numMeshes
		Mesh[numMeshes]			meshes

		u8						numMaterials
		Material[numMaterials]	materials

		u16						numEntities
		Entity[numEntities]		entities

		u16						numScripts
		Scripts[numScripts]		scripts
	}

	Material {
		"MATL"
		// u8				id

		u8					nameLength
		char[nameLength]	name

		f32[3]				color
		// Extra material data
		//	shaded? different shaders?
	}

	Mesh {
		"MESH"
		// u16					id

		u32						numVertices
		f32[numVertices * 3] 	vertices

		u32						numTriangles

		if(numVertices < 256) {
			u8 [numTriangles*3]	triangles
		}else if(numVertices < 65536) {
			u16[numTriangles*3]	triangles
		}else{
			u32[numTriangles*3]	triangles
		}

		u8[numTriangles]		triangleMaterialIDs
	}

	Entity {
		"ENTY"
		// u16				id
		// if you give an entity a name longer than 255 characters,
		//	fuck you
		u8					nameLength (can be zero)
		char[nameLength]	name
		f32[3]				position
		f32[3]				rotation
		u8					layer
		u16					parentID (can be zero)
		u16					meshID (can be zero)
		u16					scriptID (can be zero)
		u8					entityType
		u8					colliderType

		if(colliderType > 0) {
			// Collider specific size stuff
			bool			isTrigger
			bool			isKinematic
		}
	}

	// Could reuse for shader definition
	Script {
		"CODE"
		// u16				id

		u8					nameLength
		char[nameLength]	name

		u32					scriptLength
		char[scriptLength]	scriptText

		// Any metadata
	}