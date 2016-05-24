#include "voi.h"
#include "sceneloader.h"

#include <algorithm>
#include <SDL2/SDL.h>

bool InitScene(Scene* scene, const SceneData* data) {
	// NOTE: THIS IS WAY OVERKILL!!
	// It would be better to actually figure out how long the names are
	scene->nameArenaSize = (data->numMaterials + data->numEntities) * 256u;
	scene->nameArena = new char[scene->nameArenaSize];
	scene->nameArenaFree = scene->nameArena;
	std::memset(scene->nameArena, 0, scene->nameArenaSize);

	scene->numEntities = data->numEntities;
	scene->numMeshes = data->numMeshes;

	scene->entities = AllocateSceneEntities(scene->numEntities);
	scene->meshes = new Mesh[scene->numMeshes];

	if(!scene->entities || !scene->meshes) {
		return false;
	}

	std::memset(scene->materials, 0, sizeof(scene->materials));
	std::memset(scene->entities, 0, scene->numEntities * sizeof(Entity));
	std::memset(scene->meshes, 0, scene->numMeshes * sizeof(Mesh));

	// NOTE: It's possible that not all meshes will be drawn,
	//	e.g., simplified physics meshes
	// Also, static geometry should be batched
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

		SCENEPRINT("numSubmeshes: %d\n", mesh->numSubmeshes);
		for(u32 i = 0; i < mesh->numSubmeshes; i++) {
			SCENEPRINT("\tsm: start: %u\tid: %hhu\n", submeshes[i].triangleCount, submeshes[i].materialID);
		}

		glGenBuffers(2, &mesh->vbo); // I know vbo and ebo are adjacent
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glBufferData(GL_ARRAY_BUFFER, meshData->numVertices*sizeof(vec3), meshData->vertices, GL_STATIC_DRAW);

		u8 elementSize = 1<<mesh->elementType;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numTriangles*3*elementSize, meshData->triangles8, GL_STATIC_DRAW);

		// Calculate stuff for physics/portals
		vec3 minPoint{FLT_MAX}, maxPoint{FLT_MIN};

		for(u32 vid = 0; vid < meshData->numVertices; vid++) {
			auto v = meshData->vertices[vid];
			minPoint = glm::min(minPoint, v);
			maxPoint = glm::max(maxPoint, v);
		}

		mesh->center = (maxPoint + minPoint)/2.f;
		mesh->extents = (maxPoint - minPoint)/2.f;
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
		to->scale = from->scale;

		to->scene = scene;
		to->ownedByScene = true;

		// NOTE: This is here because blender applies euler rotations in ZYX by default
		//	and we swap the coord space. So gimbal lock becomes a problem.
		auto rotX = glm::angleAxis(from->rotation.x, vec3{1,0,0});
		auto rotY = glm::angleAxis(from->rotation.y, vec3{0,1,0});
		auto rotZ = glm::angleAxis(from->rotation.z, vec3{0,0,1});
		to->rotation = rotY * rotZ * rotX;

		to->parentID = from->parentID;
		to->meshID = from->meshID;
		to->entityType = from->entityType;
		to->colliderType = from->colliderType;

		if(to->meshID > 0){
			auto mesh = &scene->meshes[to->meshID-1];
			to->centerOffset = mesh->center*to->scale;
			to->extents = mesh->extents*to->scale;
		}else{
			to->centerOffset = vec3{};
			to->extents = vec3{};
		}

		if(to->entityType >= Entity::TypeNonExportable) {
			printf("Error: Entity '%.*s' has non exportable type!\n",
				(u32)to->nameLength, to->name);
			return false;	
		}

		switch(to->entityType) {
		case Entity::TypePortal:
		case Entity::TypeMirror:
			to->planeNormal = (const vec3&) from->entitySpecificData[0];
			break;

		case Entity::TypeGeometry:
		default:
			break;
		}

		auto meshData = (to->meshID>0)? &data->meshes[to->meshID-1] : nullptr;
		if(!InitEntityPhysics(to, meshData)) {
			printf("Error! Entity '%.*s' physics init failed!\n",
				(u32)to->nameLength, to->name);
			return false;
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
			printf("Warning! Portal '%.*s' only occupies zero or one layers\n", e->nameLength, e->name);
			continue;
		}

		scene->portals[scene->numPortals] = i;
		scene->numPortals++;
	}

	return true;
}

void RenderMesh(Scene* scene, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale) {
	auto program = &scene->shaders[ShaderIDDefault]; // TODO: Obvs nope
	auto mesh = &scene->meshes[meshID-1];

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

	mat4 modelMatrix = glm::translate<f32>(pos) * glm::mat4_cast(rot) * glm::scale<f32>(scale);
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

// NOTE: This could be useful elsewhere
u32 GetFarPlaneQuad(const mat4& projection) {
	static u32 farPlaneBuffer = 0;
	if(!farPlaneBuffer) {
		constexpr f32 epsilon = 1e-6;
		mat4 invProj = glm::inverse(projection);
		auto cp0 = invProj * vec4{-1,-1, 1 - epsilon, 1};
		auto cp1 = invProj * vec4{ 1,-1, 1 - epsilon, 1};
		auto cp2 = invProj * vec4{ 1, 1, 1 - epsilon, 1};
		auto cp3 = invProj * vec4{-1, 1, 1 - epsilon, 1};
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

struct PortalNode {
	u16 entityID; // 0 is root
	u16 targetLayerMask; // if root, this is layer player is in
	u16 childrenStart;
	u16 order;
	u8 childrenCount;
	u8 depth; // 0 is root
};

struct PortalGraph {
	enum{ MaxNumPortalNodes = 512 };
	PortalNode nodes[MaxNumPortalNodes];
	u16 nodeCount;
};

// Breadth first search of potentially visible portals
void ConstructPortalGraph(PortalGraph* graph, Scene* scene, u16 parentNodeID, vec3 pos, vec3 fwd, u16 firstPortalID) {
	if(graph->nodeCount >= PortalGraph::MaxNumPortalNodes) return;
	if(parentNodeID >= PortalGraph::MaxNumPortalNodes) return;
	if(firstPortalID >= scene->numPortals) return;
	
	auto parentNode = &graph->nodes[parentNodeID];
	if(parentNode->depth >= 7) return;

	u32 startCount = graph->nodeCount;
	for(u16 pid = firstPortalID; pid < scene->numPortals; pid++) {
		u32 entID = scene->portals[pid];
		auto e = &scene->entities[entID];
		if(e->entityType != Entity::TypePortal) continue;
		if(~e->layers & parentNode->targetLayerMask) continue;

		vec3 planeNormal = e->rotation * e->planeNormal;
		auto ecenter = e->position + e->rotation*e->centerOffset;
		auto gextents = e->rotation*e->extents;
		auto gext2 = glm::cross(planeNormal, gextents);
		auto diff = ecenter - pos;

		auto d0 = glm::normalize(diff);
		auto d1 = glm::normalize(diff + gextents);
		auto d2 = glm::normalize(diff - gextents);
		auto d3 = glm::normalize(diff + gext2);
		auto d4 = glm::normalize(diff - gext2);

		bool fvis = glm::dot(d0, fwd) < 0.f
			&& glm::dot(d1, fwd) < 0.f
			&& glm::dot(d2, fwd) < 0.f
			&& glm::dot(d3, fwd) < 0.f
			&& glm::dot(d4, fwd) < 0.f;
		if(fvis) continue;

		static vec3 layerColors[] = {
			vec3{1,0,0},
			vec3{1,1,0},
			vec3{0,1,0},
			vec3{0},
			vec3{0,1,1},
			vec3{0},
			vec3{0},
			vec3{0},
			vec3{0,0,1},
		};

		vec3 yoff {0, 0.04*pid, 0};
		if(parentNode->depth == 0) {
			yoff.y = -.05f;
			DebugLine(pos+yoff, ecenter, vec3{0}, layerColors[(e->layers & ~parentNode->targetLayerMask)>>1]);
		}else{
			DebugLine(pos+yoff, ecenter+yoff, layerColors[parentNode->targetLayerMask>>1], 
				layerColors[(e->layers & ~parentNode->targetLayerMask)>>1]);
		}

		auto nodeID = graph->nodeCount++;
		auto node = &graph->nodes[nodeID];
		node->entityID = entID; 
		node->targetLayerMask = e->layers & ~parentNode->targetLayerMask;
		node->childrenCount = 0;
		node->depth = parentNode->depth+1;
		node->order = pid+1;
		parentNode->childrenCount++;
	}

	u32 endCount = graph->nodeCount;
	for(; startCount < endCount; startCount++) {
		auto node = &graph->nodes[startCount];
		auto e = &scene->entities[node->entityID];
		node->childrenStart = graph->nodeCount;

		vec3 planeNormal = e->rotation * e->planeNormal;
		vec3 ecenter = e->position + e->rotation*e->centerOffset;
		auto diff = ecenter - pos;
		
		if(glm::dot(planeNormal, diff) < 0.f)
			planeNormal = -planeNormal;

		ConstructPortalGraph(graph, scene, startCount, ecenter, planeNormal, node->order);
	}
}

// Construct portal graph
// Render scene
// Mask stencil 1<<depth
// Clear stencil (masked)
//		Remove sibling portals from stencil but keep parents
// Render portal in stencil 
// 	Render target scene where stencil == (2<<depth -1)
//		This will only draw where every parent portal has been drawn
//	RECURSE
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask) {
	PortalGraph portalGraph;

	// Add all visible portals in this layer
	// For each portal
	// 		Add all visible portals from it's position in dest layer

	vec3 camFwd = cam.rotation * vec3{0,0,-1};
	vec3 camPos = cam.position;

	std::sort(scene->portals, &scene->portals[scene->numPortals], [scene, camFwd, camPos](u32 a, u32 b) {
		vec3 ad = scene->entities[a].position - camPos;
		vec3 bd = scene->entities[b].position - camPos;
		return glm::dot(ad, camFwd) < glm::dot(bd, camFwd);
	});

	// The first node represents the player
	portalGraph.nodes[0] = {0, (u16)layerMask, 1, 0, 0, 0};
	portalGraph.nodeCount = 1;
	ConstructPortalGraph(&portalGraph, scene, 0, camPos, camFwd, 0);

	static auto farPlaneBuffer = GetFarPlaneQuad(cam.projection);
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

		RenderMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale);
	}

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xff);
	glClear(GL_STENCIL_BUFFER_BIT);

	struct StackSlot {
		u32 id;
		u32 remainingChildren;
	};

	StackSlot portalStack[8] {{0, portalGraph.nodes[0].childrenCount}};
	u32 stackPos = 1;
	u32 recurseGuard = 0;

	while(stackPos > 0 && recurseGuard++ < PortalGraph::MaxNumPortalNodes+10) {
		auto parent = &portalStack[stackPos-1];
		if(parent->remainingChildren-- == 0) {
			stackPos--;
			continue;
		}

		if(stackPos >= 8) {
			parent->remainingChildren = 0;
			puts("WARNING! Render stack overflow!");
			continue;
		}

		u32 id = parent->remainingChildren + portalGraph.nodes[parent->id].childrenStart;
		if(id >= PortalGraph::MaxNumPortalNodes) {
			parent->remainingChildren = 0;
			stackPos--;
			puts("WARNING! Portal graph overflow!");
			continue;			
		}

		auto portalNode = &portalGraph.nodes[id];

		portalStack[stackPos++] = {id, portalNode->childrenCount};

		auto ent = &scene->entities[portalNode->entityID];
		if(!ent->meshID) continue;
		if(ent->entityType != Entity::TypePortal) continue;
		if(ent->flags & Entity::FlagHidden) continue;

		auto targetLayer = ent->layers & portalNode->targetLayerMask;
		if(!targetLayer) continue; // This should never really happen

		glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_CULL_FACE);

		u32 depthBit = 1<<(portalNode->depth-1);
		u32 depthMask = depthBit*2-1;

		// Prepare for drawing portal
		glStencilFunc(GL_ALWAYS, depthBit, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(~depthMask | depthBit);
		glColorMask(false,false,false,false);
		glDepthMask(false);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilMask(depthBit);

		// Write portal to stencil
		// NOTE: Polygon offset fixes z-fighting issues on some portals
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.f, -1.f);
		glDepthFunc(GL_LEQUAL);
		RenderMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale);
		glDisable(GL_POLYGON_OFFSET_FILL);

		// Clear Depth within stencil
		// 	- Disable color write
		// 	- Always write to depth
		// 	- Render quad at far plane
		// 	- Reset depth write
		glDepthFunc(GL_ALWAYS);
		glDepthMask(true);

		glStencilFunc(GL_EQUAL, depthMask, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0x0);

		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection));
		glUniformMatrix4fv(sh->modelLoc, 1, false, glm::value_ptr(mat4{}));

		// NOTE: This does nothing with posteffects on, otherwise it's clear color
		glUniform3fv(sh->materialColorLoc, 1, glm::value_ptr(vec3{.9f}));

		glBindBuffer(GL_ARRAY_BUFFER, farPlaneBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Reset render state and prepare to render target scene
		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection * viewMatrix));
		glDepthFunc(GL_LEQUAL);
		glColorMask(true,true,true,true);

		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CULL_FACE);

		vec3 ecenter = ent->position + ent->rotation * ent->centerOffset;
		vec3 dir = glm::normalize(ent->rotation * ent->planeNormal);
		vec4 plane = vec4{dir, -glm::dot(dir, ecenter)};
		if(glm::dot(dir, ecenter - cam.position) < 0.f)
			plane = -plane;

		glUniform4fv(sh->clipPlaneLoc, 1, glm::value_ptr(plane));

		for(u32 entID = 0; entID < scene->numEntities; entID++) {
			if(entID == portalNode->entityID) continue;

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

			RenderMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale);
		}
	}

	if(recurseGuard > PortalGraph::MaxNumPortalNodes) {
		puts("Warning! Portal render recurse guard hit!");
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_DISTANCE0);
	glEnable(GL_CULL_FACE);
}
