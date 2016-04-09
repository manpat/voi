#include "common.h"

#include "data.h"
#include "sceneloader.h"

void InitScene(Scene* scene, const SceneData* data) {
	// THIS IS WAY OVERKILL!!
	// It would be better to actually figure out how long the names are
	scene->nameArenaSize = (data->numMaterials + data->numEntities) * 256u;
	scene->nameArena = new char[scene->nameArenaSize];
	scene->nameArenaFree = scene->nameArena;
	std::memset(scene->nameArena, 0, scene->nameArenaSize);

	scene->numEntities = data->numEntities;
	scene->numMeshes = data->numMeshes;

	scene->entities = new Entity[scene->numEntities];
	scene->meshes = new Mesh[scene->numMeshes];

	std::memset(scene->materials, 0, sizeof(scene->materials));
	std::memset(scene->entities, 0, scene->numEntities * sizeof(Entity));
	std::memset(scene->meshes, 0, scene->numMeshes * sizeof(Mesh));

	for(u32 meshID = 0; meshID < scene->numMeshes; meshID++) {
		auto meshData = &data->meshes[meshID];
		auto mesh = &scene->meshes[meshID];
		mesh->numTriangles = meshData->numTriangles;
		mesh->elementType = GL_UNSIGNED_INT;
		if(meshData->numVertices < 256) {
			mesh->elementType = GL_UNSIGNED_BYTE;
		}else if(meshData->numVertices < 65536) {
			mesh->elementType = GL_UNSIGNED_SHORT;
		}

		// Figure out submeshes
		{	mesh->numSubmeshes = 0;
			s16 prevMatID = -1;
			for(u32 i = 0; i < mesh->numTriangles; i++) {
				s16 mat = meshData->materialIDs[i];
				if(mat > prevMatID) {
					mesh->numSubmeshes++;
					prevMatID = mat;
				}
			}
		
			auto submeshes = &mesh->submeshesInline[0];
			if(mesh->numSubmeshes > Mesh::MaxInlineSubmeshes) {
				mesh->submeshes = submeshes = new Mesh::Submesh[mesh->numSubmeshes];
			}
		
			prevMatID = -1;
			u32 matStart = 0;
		
			for(u32 i = 0; i < mesh->numTriangles; i++) {
				u8 mat = meshData->materialIDs[i];
				if(mat > prevMatID) {
					if(prevMatID != -1) {
						*submeshes++ = {matStart, (u8)prevMatID};
					}
		
					matStart = i;
					prevMatID = mat;
				}
			}
		
			*submeshes++ = {matStart, (u8)prevMatID};
		}

		auto submeshes = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
			&mesh->submeshesInline[0] : mesh->submeshes;

		printf("numSubmeshes: %d\n", mesh->numSubmeshes);
		for(u32 i = 0; i < mesh->numSubmeshes; i++) {
			printf("\tsm: start: %u\tid: %hhu\n", submeshes[i].startIndex, submeshes[i].materialID);
		}

		glGenBuffers(2, &mesh->vbo); // I know vbo and ebo are adjacent
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glBufferData(GL_ARRAY_BUFFER, meshData->numVertices*sizeof(vec3), meshData->vertices, GL_STATIC_DRAW);

		u8 elementSize;
		switch(mesh->elementType) {
			default:
			case GL_UNSIGNED_BYTE:  elementSize = mesh->elementSize = 1; break;
			case GL_UNSIGNED_SHORT: elementSize = mesh->elementSize = 2; break;
			case GL_UNSIGNED_INT: 	elementSize = mesh->elementSize = 4; break;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numTriangles*3*elementSize, meshData->triangles8, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Do material stuff
	for(u16 i = 0; i < data->numMaterials; i++){
		auto to = &scene->materials[i];
		auto from = &data->materials[i];

		std::memcpy(scene->nameArenaFree, from->name, from->nameLength);
		to->name = scene->nameArenaFree;
		scene->nameArenaFree += from->nameLength;
		*scene->nameArenaFree++ = '\0';

		to->color = from->color;
		// TODO: Shader stuff when added
	}

	// Do entity stuff
	for(u32 i = 0; i < data->numEntities; i++) {
		auto to = &scene->entities[i];
		auto from = &data->entities[i];

		std::memcpy(scene->nameArenaFree, from->name, from->nameLength);
		to->nameLength = from->nameLength;
		to->name = scene->nameArenaFree;
		scene->nameArenaFree += to->nameLength;
		*scene->nameArenaFree++ = '\0';

		to->id = i+1;
		to->flags = from->flags;

		to->layer = from->layer;
		to->position = from->position;
		to->rotation = quat(from->rotation);

		to->parentID = from->parentID;
		to->meshID = from->meshID;
		to->entityType = from->entityType;
		to->colliderType = from->colliderType;

		switch(to->entityType) {
		case Entity::TypePortal:
			to->portalInfo.targetLayer = from->entitySpecificData[0];
			break;

		case Entity::TypeGeometry:
		case Entity::TypeMirror:
		default:
			break;
		}
	}
}

void RenderScene(Scene* scene, u8 layer) {
	u32 program = scene->shaders[0].program;
	u32 materialColorLoc = glGetUniformLocation(program, "materialColor");
	u32 modelLoc = glGetUniformLocation(program, "model");
	
	for(u32 entID = 0; entID < scene->numEntities; entID++) {
		auto ent = &scene->entities[entID];
		if(!ent->meshID) continue;
		if(ent->flags & Entity::FlagHidden) continue;
		if(ent->entityType != Entity::TypeGeometry) continue; // Only render geometry, atm
		if(ent->layer != layer) continue;

		auto mesh = &scene->meshes[ent->meshID-1];

		mat4 modelMatrix = glm::translate<f32>(ent->position) * glm::mat4_cast(ent->rotation);
		glUniformMatrix4fv(modelLoc, 1, false, glm::value_ptr(modelMatrix));

		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

		u32 end = mesh->numTriangles;
		auto sms = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
			&mesh->submeshesInline[0] : mesh->submeshes;
		
		// Draw in reverse order to make getting triangle count easier
		for(s32 i = mesh->numSubmeshes-1; i >= 0; i--) {
			if(sms[i].materialID > 0) {
				auto mat = &scene->materials[sms[i].materialID-1];
				glUniform3fv(materialColorLoc, 1, glm::value_ptr(mat->color));
			}else{
				glUniform3fv(materialColorLoc, 1, glm::value_ptr(vec3{1,0,1}));
			}

			u32 begin = sms[i].startIndex;
			u32 count = end - begin;
			end = begin;

			glDrawElements(GL_TRIANGLES, count*3, mesh->elementType, 
				(void*) (begin*3ull*mesh->elementSize));
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
