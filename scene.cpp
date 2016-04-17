#include "common.h"

#include "data.h"
#include "sceneloader.h"

#include "debugdraw.h"

#include <algorithm>

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

		dprintf("numSubmeshes: %d\n", mesh->numSubmeshes);
		for(u32 i = 0; i < mesh->numSubmeshes; i++) {
			dprintf("\tsm: start: %u\tid: %hhu\n", submeshes[i].triangleCount, submeshes[i].materialID);
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
		if(!e->meshID) {
			printf("Warning! Portal '%.*s' doesn't have a mesh. That's kinda useless\n", e->nameLength, e->name);
			continue;
		}
		if(!e->layers || !(e->layers & (e->layers-1))) {
			printf("Warning! Portal '%.*s' only occupies one or fewer layers\n", e->nameLength, e->name);
			continue;
		}

		scene->portals[scene->numPortals] = i;
		scene->numPortals++;
		
		vec3 minPoint{FLT_MAX}, maxPoint{FLT_MIN};
		auto mesh = &data->meshes[e->meshID-1];

		for(u32 vid = 0; vid < mesh->numVertices; vid++) {
			auto v = mesh->vertices[vid];
			minPoint = glm::min(minPoint, v);
			maxPoint = glm::max(maxPoint, v);
		}

		e->originOffset = (maxPoint + minPoint)/2.f;
		e->extents = (maxPoint - minPoint)/2.f;
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

void ConstructPortalGraph(PortalNode* graph, u32* visiblePortals, u32 maxVisiblePortals, Scene* scene, 
	u32 layerMask, vec3 pos, vec3 fwd, vec3 pfwd, u32 start = 0) {

	if(*visiblePortals >= maxVisiblePortals) return;

	u32 processedPortals = *visiblePortals;
	for(u32 p = start; p < scene->numPortals; p++) {
		auto eID = scene->portals[p];
		auto e = &scene->entities[eID];
		if(~e->layers & layerMask) continue;

		auto ecenter = e->position + e->rotation*e->originOffset;
		auto gextents = e->rotation*e->extents;
		auto gext2 = glm::cross(e->rotation*e->planeNormal, gextents);
		auto diff = ecenter - pos;

		auto d0 = glm::normalize(diff);
		auto d1 = glm::normalize(diff + gextents);
		auto d2 = glm::normalize(diff - gextents);
		auto d3 = glm::normalize(diff + gext2);
		auto d4 = glm::normalize(diff - gext2);

		if(glm::dot(d0, pfwd) < 0.f) continue;

		DebugLine(pos+vec3{0,.1,0}, ecenter+vec3{0,.1,0}, vec3{.3}, vec3{1});

		bool fvis = glm::dot(d0, fwd) < -0.2f
			&& glm::dot(d1, fwd) < -0.2f
			&& glm::dot(d2, fwd) < -0.2f
			&& glm::dot(d3, fwd) < -0.2f
			&& glm::dot(d4, fwd) < -0.2f;
		if(fvis) continue; 
		
		DebugLine(ecenter, ecenter+fwd, {1,0,1});
		DebugLine(ecenter, ecenter+pfwd, {1,1,0});
		DebugLine(pos, ecenter, {1,0,0}, {0,1,0});

		if(*visiblePortals >= maxVisiblePortals) {
			puts("WAY TOO MANY PORTALS!");
			break;
		}
		graph[*visiblePortals].id = eID;
		graph[*visiblePortals].layerMask = layerMask;
		graph[*visiblePortals].order = p;
		++*visiblePortals;
	}

	u32 addedPortals = *visiblePortals;
	for(; processedPortals < addedPortals; processedPortals++) {
		auto eid = graph[processedPortals].id;
		auto e = &scene->entities[eid];
		auto nfwd = e->rotation*e->planeNormal;

		// Make sure portals are on right side of plane
		if(glm::dot(e->position + e->rotation*e->originOffset - pos, nfwd) < 0.f)
			nfwd = -nfwd;

		ConstructPortalGraph(graph, visiblePortals, maxVisiblePortals, scene, 
			e->layers & ~layerMask, e->position + e->rotation*e->originOffset, 
			fwd, nfwd, graph[processedPortals].order+1);
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
	vec3 camPos = cam.position;

	// camFwd = vec3{0,0,-1};
	// camPos = vec3{0,0,0};

	std::sort(scene->portals, &scene->portals[scene->numPortals], [scene, camFwd, camPos](u32 a, u32 b) {
		vec3 ad = scene->entities[a].position - camPos;
		vec3 bd = scene->entities[b].position - camPos;
		return glm::dot(ad, camFwd) < glm::dot(bd, camFwd);
	});

	ConstructPortalGraph(portalGraph, &visiblePortals, 256, scene, layerMask, camPos, camFwd, camFwd);

	for(u32 i = 0; i < visiblePortals; i++) {
		auto eid = portalGraph[i].id;
		auto e = &scene->entities[eid];
		printf("| %.*s(%x) ", e->nameLength, e->name, portalGraph[i].layerMask>>1);
	}
	printf("|\n");

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

		vec3 ecenter = ent->position + ent->rotation * ent->originOffset;
		vec3 dir = glm::normalize(ent->planeNormal * ent->rotation);
		vec4 plane = vec4{dir, -glm::dot(dir, ecenter)};
		if(glm::dot(dir, ecenter - cam.position) < 0.f)
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
