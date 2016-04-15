#include "common.h"

#include "data.h"
#include "sceneloader.h"

void InitScene(Scene* scene, const SceneData* data) {
	// NOTE: THIS IS WAY OVERKILL!!
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

		mesh->elementType = 2;
		if(meshData->numVertices < 256) {
			mesh->elementType = 0;
		}else if(meshData->numVertices < 65536) {
			mesh->elementType = 1;
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
						*submeshes++ = {i-matStart, (u8)prevMatID};
					}
		
					matStart = i;
					prevMatID = mat;
				}
			}
		
			*submeshes++ = {mesh->numTriangles-matStart, (u8)prevMatID};
		}

		auto submeshes = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
			&mesh->submeshesInline[0] : mesh->submeshes;

		printf("numSubmeshes: %d\n", mesh->numSubmeshes);
		for(u32 i = 0; i < mesh->numSubmeshes; i++) {
			printf("\tsm: start: %u\tid: %hhu\n", submeshes[i].triangleCount, submeshes[i].materialID);
		}

		glGenBuffers(2, &mesh->vbo); // I know vbo and ebo are adjacent
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glBufferData(GL_ARRAY_BUFFER, meshData->numVertices*sizeof(vec3), meshData->vertices, GL_STATIC_DRAW);

		u8 elementSize = 1<<mesh->elementType;

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

		to->layers = from->layers;
		to->position = from->position;
		to->rotation = quat(from->rotation);

		to->parentID = from->parentID;
		to->meshID = from->meshID;
		to->entityType = from->entityType;
		to->colliderType = from->colliderType;

		switch(to->entityType) {
		case Entity::TypePortal:
		case Entity::TypeMirror:
			to->planeNormal = (const vec3&) from->entitySpecificData[0];
			break;

		case Entity::TypeGeometry:
		default:
			break;
		}
	}

	// Do portal stuff
	scene->numPortals = 0;
	for(u32 i = 0; i < scene->numEntities; i++){
		auto e = &scene->entities[i];
		if(e->entityType != Entity::TypePortal) continue;

		scene->portals[scene->numPortals] = i;
		scene->numPortals++;
	}
}

void RenderMesh(Scene* scene, u16 meshID, vec3 pos, quat rot) {
	auto program = &scene->shaders[ShaderIDDefault]; // TODO: Obvs nope
	auto mesh = &scene->meshes[meshID-1];

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

	mat4 modelMatrix = glm::translate<f32>(pos) * glm::mat4_cast(rot);
	glUniformMatrix4fv(program->modelLoc, 1, false, glm::value_ptr(modelMatrix));

	auto sms = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
		&mesh->submeshesInline[0] : mesh->submeshes;

	u32 begin = 0;
	
	for(u32 i = 0; i < mesh->numSubmeshes; i++) {
		if(program->materialColorLoc) {
			if(sms[i].materialID > 0) {
				auto mat = &scene->materials[sms[i].materialID-1];
				glUniform3fv(program->materialColorLoc, 1, glm::value_ptr(mat->color));
			}else{
				glUniform3fv(program->materialColorLoc, 1, glm::value_ptr(vec3{1,0,1}));
			}
		}

		auto triangleSize = 3ull<<mesh->elementType;
		glDrawElements(GL_TRIANGLES, sms[i].triangleCount*3, Mesh::ElementTypeToGL[mesh->elementType], 
			(void*) (begin*triangleSize));

		begin += sms[i].triangleCount;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ConstructPortalGraph(PortalNode* graph, u32* visiblePortals, Scene* scene, 
	u32 layerMask, vec3 pos, vec3 fwd) {

	u32 processedPortals = *visiblePortals;
	for(u32 p = 0; p < scene->numPortals; p++) {
		auto eID = scene->portals[p];
		auto e = &scene->entities[eID];
		if(~e->layers & layerMask) continue;

		// NOTE: This is incorrect
		// The player can be looking away from the origin of the entity
		//	but still see the entity
		// Sampling multple points could be a solution
		auto diff = glm::normalize(e->position - pos);
		if(glm::dot(diff, fwd) > 0.f) {
			graph[*visiblePortals].id = eID;
			graph[*visiblePortals].layerMask = layerMask;
			++*visiblePortals;
		}
	}

	u32 addedPortals = *visiblePortals;
	for(; processedPortals < addedPortals; processedPortals++) {
		auto e = &scene->entities[graph[processedPortals].id];
		ConstructPortalGraph(graph, visiblePortals, scene, e->layers & ~layerMask, e->position, fwd);
	}
}

u32 GetFarPlaneQuad(mat4 projection) {
	static u32 farPlaneBuffer = 0;
	if(!farPlaneBuffer) {
		mat4 invProj = glm::inverse(projection);
		auto cp0 = invProj * vec4{-1,-1, 1 - 1e-6, 1};
		auto cp1 = invProj * vec4{ 1,-1, 1 - 1e-6, 1};
		auto cp2 = invProj * vec4{ 1, 1, 1 - 1e-6, 1};
		auto cp3 = invProj * vec4{-1, 1, 1 - 1e-6, 1};
		auto p0 = vec3{cp0/cp0.w};
		auto p1 = vec3{cp1/cp1.w};
		auto p2 = vec3{cp2/cp2.w};
		auto p3 = vec3{cp3/cp3.w};

		vec3 points[] = {
			p0, p1, p2,
			p0, p2, p3,
		};

		glGenBuffers(1, &farPlaneBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, farPlaneBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	return farPlaneBuffer;
}

void RenderScene(Scene* scene, const Camera& cam, u32 layerMask) {
	auto farPlaneBuffer = GetFarPlaneQuad(cam.projection);
	auto sh = &scene->shaders[ShaderIDDefault];

	mat4 viewMatrix = glm::mat4_cast(glm::inverse(cam.rotation)) * glm::translate<f32>(-cam.position);
	glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection * viewMatrix));

	for(u32 entID = 0; entID < scene->numEntities; entID++) {
		auto ent = &scene->entities[entID];
		if(!ent->meshID) continue;
		if(ent->flags & Entity::FlagHidden) continue;
		if(~ent->layers & layerMask) continue;

		// Render both sides of portals and mirrors
		if(ent->entityType == Entity::TypePortal
		|| ent->entityType == Entity::TypeMirror) {
			glDisable(GL_CULL_FACE);
		}else{
			glEnable(GL_CULL_FACE);
		}

		RenderMesh(scene, ent->meshID, ent->position, ent->rotation);
	}

	PortalNode portalGraph[256];
	u32 visiblePortals = 0;

	// Add all visible portals in this layer
	// For each portal
	// 		Add all visible portals from it's position in dest layer

	vec3 camFwd = cam.rotation * vec3{0,0,-1};
	ConstructPortalGraph(portalGraph, &visiblePortals, scene, layerMask, cam.position, camFwd);

	glEnable(GL_STENCIL_TEST);

	for(u32 graphPos = 0; graphPos < visiblePortals; graphPos++) {
		auto portal = &portalGraph[graphPos];
		auto ent = &scene->entities[portal->id];
		if(!ent->meshID) continue;
		if(ent->entityType != Entity::TypePortal) continue;
		if(ent->flags & Entity::FlagHidden) continue;

		auto targetLayer = ent->layers & ~portal->layerMask;

		glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_CULL_FACE);

		// Write portal to stencil
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		// glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xff);
		glClear(GL_STENCIL_BUFFER_BIT);

		// Render portal
		glDepthFunc(GL_EQUAL); // HACK: Assumes nothing else lines up with portal, can cause artifacts
		RenderMesh(scene, ent->meshID, ent->position, ent->rotation);

		// Clear Depth within stencil
		// 	- Disable color write
		// 	- Always write to depth
		// 	- Render quad at far plane
		// 	- Reset depth write
		glDepthFunc(GL_ALWAYS);
		glStencilFunc(GL_EQUAL, 1, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0x0);

		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection));
		glUniformMatrix4fv(sh->modelLoc, 1, false, glm::value_ptr(mat4{}));
		glUniform3fv(sh->materialColorLoc, 1, glm::value_ptr(vec3{.1f})); ////////////// Clear color

		glBindBuffer(GL_ARRAY_BUFFER, farPlaneBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Reset render state
		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection * viewMatrix));
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CULL_FACE);

		vec3 dir = glm::normalize(ent->planeNormal * ent->rotation);
		vec4 plane = vec4{dir, -glm::dot(dir, ent->position)};
		if(glm::dot(dir, ent->position - cam.position) < 0.f)
			plane = -plane;

		glUniform4fv(sh->clipPlaneLoc, 1, glm::value_ptr(plane));

		for(u32 entID = 0; entID < scene->numEntities; entID++) {
			if(entID == portal->id) continue;

			auto ent = &scene->entities[entID];
			if(!ent->meshID) continue;
			if(ent->flags & Entity::FlagHidden) continue;
			if(~ent->layers & targetLayer) continue;

			if(ent->entityType == Entity::TypePortal
			|| ent->entityType == Entity::TypeMirror) {
				glDisable(GL_CULL_FACE);
			}else{
				glEnable(GL_CULL_FACE);
			}

			RenderMesh(scene, ent->meshID, ent->position, ent->rotation);
		}
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_DISTANCE0);
	glEnable(GL_CULL_FACE);
}
