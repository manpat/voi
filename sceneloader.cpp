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

template<class El>
void SortMeshData(RawMeshData* md);

SceneData LoadSceneData(const char* fname) {
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

	SceneData scene{};

	u8* it = data;
	if(*it++ != 'V'
	|| *it++ != 'O'
	|| *it++ != 'I') {
		puts("Invalid filestamp!");
		return {};
	}

	u8 version = *it++;
	assert(version == 1, "Unknown scene format version!");

	scene.numMeshes = Read<u16>(&it);
	scene.meshes = new RawMeshData[scene.numMeshes];
	printf("numMeshes: %hi\n", scene.numMeshes);

	for(u16 i = 0; i < scene.numMeshes; i++) {
		if(!CheckStamp(&it, "MESH")) goto error;

		auto mesh = &scene.meshes[i];
		mesh->numVertices = Read<u32>(&it);
		mesh->vertices = new vec3[mesh->numVertices];

		std::memcpy(mesh->vertices, it, mesh->numVertices*sizeof(vec3));
		it += mesh->numVertices*sizeof(vec3);

		mesh->numTriangles = Read<u32>(&it);

		if(mesh->numVertices < 256) {
			mesh->triangles8 = new u8[mesh->numTriangles*3];
			std::memcpy(mesh->triangles8, it, mesh->numTriangles*3);
			it += mesh->numTriangles*3;

		}else if(mesh->numVertices < 65536) {
			mesh->triangles16 = new u16[mesh->numTriangles*3];
			std::memcpy(mesh->triangles16, it, mesh->numTriangles*6);
			it += mesh->numTriangles*3*2;

		}else{
			mesh->triangles32 = new u32[mesh->numTriangles*3];
			std::memcpy(mesh->triangles32, it, mesh->numTriangles*12);
			it += mesh->numTriangles*3*4;
		}

		mesh->materialIDs = new u8[mesh->numTriangles];
		std::memcpy(mesh->materialIDs, it, mesh->numTriangles);
		it += mesh->numTriangles;

		printf("\tnumVertices: %u\n", mesh->numVertices);
		printf("\tnumTriangles: %u\n", mesh->numTriangles);

		for(u32 j = 0; j < mesh->numVertices; j++) {
			auto v = &mesh->vertices[j];
			printf("\t\tv: (%.1f, %.1f, %.1f)\n", v->x, v->y, v->z);
		}

		if(mesh->numVertices < 256) {
			SortMeshData<u8>(mesh);
			for(u32 j = 0; j < mesh->numTriangles; j++) {
				auto v = &mesh->triangles8[j*3];
				auto m = mesh->materialIDs[j];
				printf("\t\tf: (%hhu, %hhu, %hhu) %hhu\n", v[0], v[1], v[2], m);
			}

		}else if(mesh->numVertices < 65536) {
			SortMeshData<u16>(mesh);
			for(u32 j = 0; j < mesh->numTriangles; j++) {
				auto v = &mesh->triangles16[j*3];
				auto m = mesh->materialIDs[j];
				printf("\t\tf: (%hu, %hu, %hu) %hhu\n", v[0], v[1], v[2], m);
			}

		}else{
			SortMeshData<u32>(mesh);
			for(u32 j = 0; j < mesh->numTriangles; j++) {
				auto v = &mesh->triangles32[j*3];
				auto m = mesh->materialIDs[j];
				printf("\t\tf: (%u, %u, %u) %hhu\n", v[0], v[1], v[2], m);
			}
		}

		puts("");
	}

	scene.numMaterials = *it++;
	scene.materials = new MaterialData[scene.numMaterials];
	printf("numMaterials: %hhu\n", scene.numMaterials);

	for(u8 i = 0; i < scene.numMaterials; i++) {
		if(!CheckStamp(&it, "MATL")) goto error;

		u8 nameLength = scene.materials[i].nameLength = *it++;
		std::memcpy(scene.materials[i].name, it, nameLength);
		printf("\tmaterialName: %.*s\n", nameLength, it);
		it += nameLength;
		vec3 color = scene.materials[i].color = Read<vec3>(&it);
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

	return scene;
}

template<class El>
void SortMeshData(RawMeshData* md) {
	auto ms = md->materialIDs;
	auto ts = (El*) md->triangles8;
	u32 count = md->numTriangles;

	auto swapElements = [ms,ts] (u32 a, u32 b) {
		std::swap(ms[a], ms[b]);
		std::swap(ts[a*3+0], ts[b*3+0]);
		std::swap(ts[a*3+1], ts[b*3+1]);
		std::swap(ts[a*3+2], ts[b*3+2]);
	};

	auto siftdown = [swapElements] (u8* a, u32 start, u32 end) {
		u32 root = start;

		// While root has a child
		while((root*2+1) <= end) {
			u32 left = (root*2+1);
			u32 right = left+1;
			u32 swap = root;

			if(a[swap] < a[left])
				swap = left;

			// If right child exists and is greater
			if(right <= end && a[swap] < a[right])
				swap = right;

			if(swap != root) {
				swapElements(root, swap);
				root = swap;
			}else{
				return;
			}
		}
	};
	
	// Make heap
	u32 end = count-1;
	s64 start = (end-1)/2;
	while(start >= 0) {
		siftdown(ms, start, end);
		start--;
	}

	// Sort
	while(end > 0) {
		swapElements(end, 0);
		end--;
		siftdown(ms, 0, end);
	}
}

void FreeSceneData(SceneData* scene) {
	for(u32 i = 0; i < scene->numMeshes; i++) {
		auto mesh = &scene->meshes[i];
		delete[] mesh->vertices;
		delete[] mesh->triangles8;
		delete[] mesh->materialIDs;
	}

	delete[] scene->meshes;
	delete[] scene->materials;
	// TODO: The rest when it exists
}
