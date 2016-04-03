#include "sceneloader.h"
#include <fstream>

#undef assert
void assert(bool c, const char* s) {
	if(!c) {
		puts(s);
		std::exit(0);
	}
}

template<class T>
T Read(u8** it) {
	T v = *(T*)*it;
	*it += sizeof(T);
	return v;
}

bool CheckStamp(u8** it, const char* stamp) {
	u8* i = *it;
	*it += 4;
	if((i[0] == stamp[0])
	&&(	i[1] == stamp[1])
	&&(	i[2] == stamp[2])
	&&(	i[3] == stamp[3])) {
		return true;
	}

	printf("Error! Expected %.*s stamp but didn't get one!\n", 4, stamp);
	return false;
}

struct Scene {
	u16 numMeshes;
	u8  numMaterials;
	u16 numEntities;
	u16 numScripts;
};

void LoadScene(const char* fname) {
	printf("Loading %s ...\n", fname);

	u8* data = nullptr;
	u64 size = 0;

	{	std::ifstream file{fname, file.binary};
		file.seekg(0, file.end);
		size = file.tellg();
		data = new u8[size];
		file.seekg(0, file.beg);
		file.read((char*)data, size);
	}

	Scene scene;

	u8* it = data;
	if(*it++ != 'V'
	|| *it++ != 'O'
	|| *it++ != 'I') {
		puts("Invalid filestamp!");
		return;
	}

	u8 version = *it++;
	assert(version == 1, "Unknown scene format version!");

	scene.numMeshes = Read<u16>(&it);
	printf("numMeshes: %hi\n", scene.numMeshes);

	for(u16 i = 0; i < scene.numMeshes; i++) {
		if(!CheckStamp(&it, "MESH")) goto error;

		u32 numVertices = Read<u32>(&it);
		it += numVertices*sizeof(vec3);
		u32 numTriangles = Read<u32>(&it);

		// Skip indices
		if(numVertices < 256) {
			it += numTriangles*3;
		}else if(numVertices < 65536) {
			it += numTriangles*3*2;
		}else{
			it += numTriangles*3*4;
		}

		// Skip materials
		it += numTriangles;

		printf("\tnumVertices: %u\n", numVertices);
		printf("\tnumTriangles: %u\n\n", numTriangles);
	}

	scene.numMaterials = *it++;
	printf("numMaterials: %hhu\n", scene.numMaterials);

	for(u8 i = 0; i < scene.numMaterials; i++) {
		if(!CheckStamp(&it, "MATL")) goto error;

		u8 nameLength = *it++;
		printf("\tmaterialName: %.*s\n", nameLength, it);
		it += nameLength;
		vec3 color = Read<vec3>(&it);
		printf("\tmaterialColor: (%.2f, %.2f, %.2f)\n\n", color.r, color.g, color.b);
	}

	scene.numEntities = Read<u16>(&it);
	printf("numEntities: %hu\n", scene.numEntities);
	for(u16 i = 0; i < scene.numEntities; i++) {
		if(!CheckStamp(&it, "ENTY")) goto error;
		
		u8 nameLength = *it++;
		printf("\tentityName: %.*s\n", nameLength, it);
		it += nameLength;

		vec3 position = Read<vec3>(&it);
		vec3 rotation = Read<vec3>(&it);

		u16 parentID = Read<u16>(&it);
		u16 meshID = Read<u16>(&it);
		u16 scriptID = Read<u16>(&it);
		u8  entityType = *it++;
		u8  colliderType = *it++;

		printf("\tposition: (%.1f, %.1f, %.1f)\n", position.x, position.y, position.z);
		printf("\trotation: (%.1f, %.1f, %.1f)\n", rotation.x, rotation.y, rotation.z);
		printf("\tparentID: %hu\n\tmeshID: %hu\n", parentID, meshID);
		printf("\tscriptID: %hu\n", scriptID);
		printf("\tentityType: %hhu\n\tcolliderType: %hhu\n\n", entityType, colliderType);
	}

	scene.numScripts = Read<u16>(&it);
	printf("numScripts: %hu\n", scene.numScripts);
	for(u16 i = 0; i < scene.numScripts; i++) {
		if(!CheckStamp(&it, "CODE")) goto error;

		u8 nameLength = *it++;
		printf("\tscriptName: %.*s\n", (u32)nameLength, it);
		it += nameLength;

		u32 scriptLength = Read<u32>(&it);
		printf("\tscriptText:\n----------------\n%.*s\n-----------------\n\n", 
			scriptLength, it);
		it += scriptLength;
	}

error:
	delete[] data;
	puts("Done.");
}
