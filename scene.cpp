#include "voi.h"
#include "ext/stb.h"
#include "sceneloader.h"

#include <map>
#include <vector>
#include <limits>
#include <algorithm>
#include <SDL2/SDL.h>

void RenderSceneMesh(Scene*, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale = vec3{1,1,1}, bool ignoreFog = false);
void RenderSceneMeshAlpha(Scene*, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale = vec3{1,1,1}, f32 alpha = 1.f);

static u32 vertVBO = 0;
static u32 idxVBO = 0;

bool InitScene(Scene* scene, const SceneData* data) {
	// NOTE: THIS IS WAY OVERKILL!!
	// It would be better to actually figure out how long the names are
	scene->nameArenaSize = (data->numEntities) * 256u;
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

	if(!vertVBO) glGenBuffers(1, &vertVBO);
	if(!idxVBO) glGenBuffers(1, &idxVBO);

	std::vector<vec3> vertices;
	std::vector<u32> indices;

	// NOTE: It's possible that not all meshes will be drawn,
	//	e.g., simplified physics meshes
	// Also, static geometry should be batched
	for(u32 meshID = 0; meshID < scene->numMeshes; meshID++) {
		auto meshData = &data->meshes[meshID];
		auto mesh = &scene->meshes[meshID];
		mesh->numTriangles = meshData->numTriangles;

		// mesh->elementType = 2;
		// if(meshData->numVertices < 256) {
		// 	mesh->elementType = 0;
		// }else if(meshData->numVertices < 65536) {
		// 	mesh->elementType = 1;
		// }

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

		mesh->indexOffset = indices.size();
		u32 vertOffset = vertices.size();

		vertices.insert(vertices.end(), meshData->vertices, meshData->vertices+meshData->numVertices);

		switch(meshData->elementType) {
		default:
		case 0:
			for(u32 i = 0; i < meshData->numTriangles*3u; i++)
				indices.push_back(vertOffset + meshData->triangles8[i]);
			break;
		case 1:
			for(u32 i = 0; i < meshData->numTriangles*3u; i++)
				indices.push_back(vertOffset + meshData->triangles16[i]);
			break;
		case 2:
			for(u32 i = 0; i < meshData->numTriangles*3u; i++)
				indices.push_back(vertOffset + meshData->triangles32[i]);
			break;
		}

		// Calculate stuff for physics/portals
		vec3 minPoint{constant::infinity}, maxPoint{-constant::infinity};

		for(u32 vid = 0; vid < meshData->numVertices; vid++) {
			auto v = meshData->vertices[vid];
			minPoint = glm::min(minPoint, v);
			maxPoint = glm::max(maxPoint, v);
		}

		mesh->center = (maxPoint + minPoint)/2.f;
		mesh->extents = (maxPoint - minPoint)/2.f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Do material stuff
	for(u16 i = 0; i < data->numMaterials; i++){
		scene->materials[i] = data->materials[i];
	}

	std::map<u32, s32> loadedScripts {};
	auto GetScript = [&loadedScripts](const char* fname) {
		static char buf[512];
		std::snprintf(buf, 512, "scripts/%s", fname);
		s32* script = &loadedScripts[stb_hash(fname)];
		s32 tmpscript = *script;
		if(!tmpscript) tmpscript = LoadScript(buf);
		return *script = tmpscript;
	};

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
			LogError("Error! Entity '%.*s' has non exportable type!\n",
				(u32)to->nameLength, to->name);
			return false;
		}

		if(from->initCallbackLen) {
			char* scriptFile = from->initCallback;
			scriptFile[std::min<u16>(from->initCallbackLen, 255)] = 0;
			auto action = std::strchr(scriptFile, ':');

			if(action) {
				*action++ = 0;
				if(auto script = GetScript(scriptFile)){
					to->initCallback = GetCallbackFromScript(script, action);
				}
			}else{
				LogError("Warning! Update callback for %.*s missing action\n", 
					(u32)to->nameLength, to->name);
			}
		}

		if(from->updateCallbackLen) {
			char* scriptFile = from->updateCallback;
			scriptFile[std::min<u16>(from->updateCallbackLen, 255)] = 0;
			auto action = std::strchr(scriptFile, ':');

			if(action) {
				*action++ = 0;
				if(auto script = GetScript(scriptFile)){
					to->updateCallback = GetCallbackFromScript(script, action);
				}
			}else{
				LogError("Warning! Update callback for %.*s missing action\n", 
					(u32)to->nameLength, to->name);
			}
		}

		// NOTE: Gets around strict aliasing rule (actually probably not but stops the warning)
		// 	Would be better to either figure out exactly why this breaks the strict aliasing rule
		//	and fix it, or disable optimisations based on the strict aliasing rule (-fno-strict-alias)
		char* entitySpecificData = (char*)from->entitySpecificData;

		switch(to->entityType) {
		case Entity::TypePortal:
		case Entity::TypeMirror:
			to->planeNormal = glm::normalize((const vec3&) entitySpecificData[0]);
			break;

		case Entity::TypeInteractive: {
			u8 actionlen = *entitySpecificData++;
			*(entitySpecificData+actionlen) = 0;
			auto scriptFile = entitySpecificData;
			auto action = std::strchr(scriptFile, ':');
			if(!action){
				LogError("Warning! Frob callback for '%.*s' missing action\n", 
					(u32)to->nameLength, to->name);
			}else{
				*action++ = 0;
				if(auto script = GetScript(scriptFile)){
					to->interact.frobCallback = GetCallbackFromScript(script, action);
				}
			}

			to->flags |= Entity::FlagStatic; // TODO: Make kinematic instead, just in case we want to move 'em
		} break;

		case Entity::TypeTrigger:{
			to->flags |= Entity::FlagStatic; // TODO: Make kinematic instead, just in case we want to move 'em
			char actionBuffer[257];

			u8 enterlen = *entitySpecificData++;
			std::memcpy(actionBuffer, entitySpecificData, enterlen);
			actionBuffer[enterlen] = 0;

			if(auto enterAction = std::strchr(actionBuffer, ':')) {
				*enterAction++ = 0;
				if(auto script = GetScript(actionBuffer)){
					to->trigger.enterCallback = GetCallbackFromScript(script, enterAction);
				}
			}else if(enterlen){
				LogError("Warning! Enter callback for '%.*s' missing action\n", 
					(u32)to->nameLength, to->name);
			}

			entitySpecificData += enterlen;
			u8 leavelen = *entitySpecificData++;
			std::memcpy(actionBuffer, entitySpecificData, leavelen);
			actionBuffer[leavelen] = 0;

			if(auto leaveAction = std::strchr(actionBuffer, ':')) {
				*leaveAction++ = 0;
				if(auto script = GetScript(actionBuffer)){
					to->trigger.leaveCallback = GetCallbackFromScript(script, leaveAction);
				}
			}else if(leavelen){
				LogError("Warning! Leave callback for '%.*s' missing action\n", 
					(u32)to->nameLength, to->name);
			}
		} break;
			
		case Entity::TypeGeometry:
		default:
			break;
		}

		if(to->entityType == Entity::TypePortal) {
			to->colliderType = ColliderCube;
			to->flags |= Entity::FlagStatic; // TODO: Make kinematic instead, just in case we want to move 'em
			to->extents += to->planeNormal * 0.05f;
		}

		auto meshData = (to->meshID>0)? &data->meshes[to->meshID-1] : nullptr;
		if(!InitEntityPhysics(to, meshData)) {
			LogError("Error! Entity '%.*s' physics init failed!\n",
				(u32)to->nameLength, to->name);
			return false;
		}

		InitEntity(to);
	}

	// We don't need to hold onto a script reference once we have references 
	//	to the callbacks
	for(auto sc: loadedScripts)
		UnloadScript(sc.second);

	// Do portal stuff
	scene->numPortals = 0;
	for(u32 i = 0; i < scene->numEntities; i++){
		auto e = &scene->entities[i];
		if(e->entityType != Entity::TypePortal && e->entityType != Entity::TypeMirror) continue;
		if(!e->meshID) {
			LogError("Warning! Portal/Mirror '%.*s' doesn't have a mesh. That's kinda useless\n", e->nameLength, e->name);
			continue;
		}
		if(e->entityType == Entity::TypePortal && (!e->layers || !(e->layers & (e->layers-1)))) {
			LogError("Warning! Portal '%.*s' only occupies zero or one layers\n", e->nameLength, e->name);
			continue;
		}

		scene->portals[scene->numPortals] = i;
		scene->numPortals++;
	}

	return true;
}

void DeinitScene(Scene* scene) {
	for(u32 i = 0; i < scene->numEntities; i++)
		DeinitEntity(&scene->entities[i]);

	for(u32 i = 0; i < scene->numEntities; i++)
		DeinitEntityPhysics(&scene->entities[i]);

	FreeSceneEntities(scene->entities);
	DeinitPhysics(&scene->physicsContext);

	delete[] scene->nameArena;
	delete[] scene->meshes;

	std::memset(scene, 0, sizeof(Scene));
}

// NOTE: This assumes vertVBO and idxVBO are bound
void RenderSceneMesh(Scene* scene, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale, bool ignoreFog) {
	RenderSceneMeshAlpha(scene, meshID, pos, rot, scale, ignoreFog?0:1);
}

void RenderSceneMeshAlpha(Scene* scene, u16 meshID, const vec3& pos, const quat& rot, const vec3& scale, f32 alpha) {
	auto program = GetNamedShaderProgram(ShaderIDDefault); // TODO: Obvs nope
	auto mesh = &scene->meshes[meshID-1];

	mat4 modelMatrix = glm::translate<f32>(pos) * glm::mat4_cast(rot) * glm::scale<f32>(scale);
	glUniformMatrix4fv(program->modelLoc, 1, false, glm::value_ptr(modelMatrix));

	auto sms = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
		&mesh->submeshesInline[0] : mesh->submeshes;

	u32 begin = 0;
	
	for(u32 i = 0; i < mesh->numSubmeshes; i++) {
		if(program->materialColorLoc) {
			if(sms[i].materialID > 0) {
				auto mat = scene->materials[sms[i].materialID-1];
				glUniform4fv(program->materialColorLoc, 1, glm::value_ptr(vec4{mat, alpha}));
			}else{
				glUniform4fv(program->materialColorLoc, 1, glm::value_ptr(vec4{1,0,1, alpha}));
			}
		}

		glDrawElements(GL_TRIANGLES, sms[i].triangleCount*3, GL_UNSIGNED_INT, (void*) ((begin*3ull + mesh->indexOffset)*sizeof(u32)));

		begin += sms[i].triangleCount;
	}
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
void ConstructPortalGraph(PortalGraph* graph, Scene* scene, u16 parentNodeID, vec3 pos, vec3 fwd, u16 firstPortalID, bool inMirror = false) {
	DebugLine(pos, pos+fwd);

	if(graph->nodeCount >= PortalGraph::MaxNumPortalNodes) return;
	if(parentNodeID >= PortalGraph::MaxNumPortalNodes) return;
	if(firstPortalID >= scene->numPortals) return;
	
	auto parentNode = &graph->nodes[parentNodeID];
	if(parentNode->depth >= 7) return;

	u32 startCount = graph->nodeCount;
	u16 begin = firstPortalID;
	u16 end = scene->numPortals;

	for(u16 pid = begin; pid < end; pid++) {
		u32 entID = scene->portals[pid];
		auto e = &scene->entities[entID];
		bool isPortal = e->entityType == Entity::TypePortal;
		bool isMirror = e->entityType == Entity::TypeMirror;

		if(!isPortal && !isMirror) continue;
		if(isMirror && inMirror) continue;

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
		vec3 planeNormal = e->rotation * e->planeNormal;
		auto ecenter = e->position + e->rotation*e->centerOffset;

		if(isPortal) {
			if(~e->layers & parentNode->targetLayerMask) continue;

			const auto gextents = e->rotation*e->extents;
			const auto gext2 = glm::cross(planeNormal, gextents);
			const auto diff = ecenter - pos;

			const auto d0 = diff;
			const auto d1 = diff + gextents;
			const auto d2 = diff - gextents;
			const auto d3 = diff + gext2;
			const auto d4 = diff - gext2;

			const bool fvis = glm::dot(d0, fwd) < 0.f
				&& glm::dot(d1, fwd) < 0.f
				&& glm::dot(d2, fwd) < 0.f
				&& glm::dot(d3, fwd) < 0.f
				&& glm::dot(d4, fwd) < 0.f;
			if(fvis) continue;

			const auto nodeID = graph->nodeCount++;
			auto node = &graph->nodes[nodeID];
			node->entityID = entID;
			node->targetLayerMask = e->layers & ~parentNode->targetLayerMask;
			node->childrenCount = 0;
			node->depth = parentNode->depth+1;
			node->order = pid+1;
			parentNode->childrenCount++;

		}else if(isMirror) {
			if(!(e->layers & parentNode->targetLayerMask)) continue;

			const auto nodeID = graph->nodeCount++;
			auto node = &graph->nodes[nodeID];
			node->entityID = entID; 
			node->targetLayerMask = e->layers & parentNode->targetLayerMask;
			node->childrenCount = 0;
			node->depth = parentNode->depth+1;
			node->order = 0; //pid+1;
			parentNode->childrenCount++;
		}

		if(parentNode->depth == 0) {
			yoff.y = -.05f;
			DebugLine(pos+yoff, ecenter, vec3{0}, layerColors[(e->layers & ~parentNode->targetLayerMask)>>1]);
		}else{
			DebugLine(pos+yoff, ecenter+yoff, layerColors[parentNode->targetLayerMask>>1], 
				layerColors[(e->layers & ~parentNode->targetLayerMask)>>1]);
		}
	}

	u32 endCount = graph->nodeCount;
	for(; startCount < endCount; startCount++) {
		auto node = &graph->nodes[startCount];
		auto e = &scene->entities[node->entityID];
		node->childrenStart = graph->nodeCount;

		vec3 planeNormal = e->rotation * e->planeNormal;
		vec3 ecenter = e->position + e->rotation*e->centerOffset;
		auto diff = ecenter - pos;
		
		bool isMirror = e->entityType == Entity::TypeMirror;

		if(glm::dot(planeNormal, diff) * (isMirror?-1:1) < 0.f)
			planeNormal = -planeNormal;

		ConstructPortalGraph(graph, scene, startCount, ecenter, planeNormal, node->order, isMirror || inMirror);
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
	auto intPtl = GetEntity(cam.intersectingPortalId);

	static u32 portalClipPlaneBuffer = 0;
	if(!portalClipPlaneBuffer) {
		glGenBuffers(1, &portalClipPlaneBuffer);
	}

	std::sort(scene->portals, &scene->portals[scene->numPortals], [scene, camFwd, camPos](u32 a, u32 b) {
		auto ea = &scene->entities[a];
		auto eb = &scene->entities[b];
		vec3 ad = ea->position - camPos;
		vec3 bd = eb->position - camPos;
		return glm::dot(ad, camFwd)	< glm::dot(bd, camFwd);
	});

	// std::sort(scene->portals, &scene->portals[scene->numPortals], [scene](u32 a, u32 b) {
	// 	auto ea = &scene->entities[a];
	// 	auto eb = &scene->entities[b];
	// 	return (ea->entityType == Entity::TypeMirror) > (eb->entityType == Entity::TypeMirror);
	// });

	// The first node represents the player
	portalGraph.nodes[0] = {0, (u16)layerMask, 1, 0, 0, 0};
	portalGraph.nodeCount = 1;
	ConstructPortalGraph(&portalGraph, scene, 0, camPos, camFwd, 0);

	auto sh = GetNamedShaderProgram(ShaderIDDefault);

	// mat4 viewMatrix = glm::mat4_cast(glm::inverse(cam.rotation)) * glm::translate<f32>(-cam.position);
	mat4 viewProjection = cam.projection * cam.view;
	glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(viewProjection));

	if(intPtl) {
		// http://stackoverflow.com/questions/6408670/line-of-intersection-between-two-planes
		auto ptlNorm = intPtl->rotation*intPtl->planeNormal;
		vec3 ptlPos = intPtl->position + intPtl->rotation * intPtl->centerOffset;
		auto intersectDir = glm::cross(camFwd, ptlNorm);
		f32 det = glm::length2(intersectDir);
		
		// If det == 0, no intersect, bail
		// NOTE: This epsilon could be made larger perhaps as an optimisation
		if(glm::abs(det) > 1e-4) {
			auto near = camPos + camFwd*cam.nearDist;

			f32 ptld = -glm::dot(ptlPos, ptlNorm);
			f32 camd = -glm::dot(near, camFwd);

			auto point = (glm::cross(intersectDir, ptlNorm) * camd +
				glm::cross(camFwd, intersectDir) * ptld) / det;

			intersectDir = glm::normalize(intersectDir);

			DebugLine(near, point);
			DebugLine(point-intersectDir*100.f, point+intersectDir*100.f, vec3{1, vec2{glm::length(intersectDir)}});

			auto projpoint = vec3{cam.view * vec4{point, 1}};
			auto projdir = vec3{cam.view * vec4{intersectDir, 0}};
			auto perpdir = glm::normalize(glm::cross(projdir, vec3{0, 0, -1}));
			if(glm::dot(ptlNorm, ptlPos - cam.position) < 0.f)
				perpdir = -perpdir;

			vec3 intPoint{projpoint};
			if(glm::abs(projdir.x) < glm::abs(projdir.y)) {
				// f(x) = mx + c 
				// m = dy/dx
				// c = f(x) - mx
				// 0 = mx + c 
				// -c/m = x
				f32 m = projdir.y/projdir.x;
				f32 c = intPoint.y - m*intPoint.x;

				intPoint.x = -c/m;
				intPoint.y = 0.f;

			}else{
				f32 m = projdir.x/projdir.y;
				f32 c = intPoint.x - m*intPoint.y;

				intPoint.y = -c/m;
				intPoint.x = 0.f;
			}

			// LogError("(%7.2f %7.2f) -> (%7.2f %7.2f) (%3.2f %3.2f)\n", projpoint.x, projpoint.y, intPoint.x, intPoint.y, projdir.x, projdir.y);

			intPoint.z = -cam.nearDist - 1e-5;
			projdir.z = 0.f;
			perpdir.z = 0.f;

			vec3 points[] = {
				intPoint + projdir*10.f,
				intPoint + projdir*10.f + perpdir*10.f,
				intPoint - projdir*10.f + perpdir*10.f,
				intPoint - projdir*10.f,
			};

			glBindBuffer(GL_ARRAY_BUFFER, portalClipPlaneBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}else{
			// LogError("NO INTERSECT\n");
			// NOTE: None of this has been tested.
			// 	This is very much a corner case

			if(glm::dot(camFwd, ptlPos - cam.position) < 0.f){
				intPtl = nullptr;
			}else{
				mat4 invProj = glm::inverse(cam.projection);
				auto cp0 = invProj * vec4{-1,-1, 1e-6, 1};
				auto cp1 = invProj * vec4{ 1,-1, 1e-6, 1};
				auto cp2 = invProj * vec4{ 1, 1, 1e-6, 1};
				auto cp3 = invProj * vec4{-1, 1, 1e-6, 1};

				vec3 points[] = {
					vec3{cp0/cp0.w},
					vec3{cp1/cp1.w},
					vec3{cp2/cp2.w},
					vec3{cp3/cp3.w},
				};

				glBindBuffer(GL_ARRAY_BUFFER, portalClipPlaneBuffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STREAM_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glDisable(GL_BLEND);

	for(u32 entID = 0; entID < scene->numEntities; entID++) {
		auto ent = &scene->entities[entID];
		if(!ent->meshID) continue;
		if(ent->flags & Entity::FlagHidden) continue;
		if(~ent->layers & layerMask) continue;

		if(ent->entityType == Entity::TypeTrigger) continue;

		// Render both sides of portals and mirrors
		if(ent->entityType == Entity::TypePortal
		|| ent->entityType == Entity::TypeMirror) {
			glDisable(GL_CULL_FACE);
		}else if(ent->flags & Entity::FlagDoubleSided){
			glDisable(GL_CULL_FACE);
		}else{
			glEnable(GL_CULL_FACE);
		}

		RenderSceneMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale, bool(ent->flags&Entity::FlagIgnoreFog));
	}

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xff);
	glClear(GL_STENCIL_BUFFER_BIT);

	struct StackSlot {
		mat4 viewProjection;
		u32 graphIdx;
		u32 remainingChildren;
		bool isMirror;
	};

	StackSlot portalStack[8] {{viewProjection, 0, portalGraph.nodes[0].childrenCount, false}};
	u32 stackPos = 1;
	u32 recurseGuard = 0;
	bool isInMirror = false;

	// NOTE: This limiting condition is a shot in the dark but seems to work mostly
	while(stackPos > 0 && recurseGuard++ < PortalGraph::MaxNumPortalNodes+10) {
		auto parent = &portalStack[stackPos-1];
		if(parent->remainingChildren-- == 0) {
			// Pop stack frame
			if(parent->isMirror) {
				isInMirror = false;
			}
			stackPos--;
			continue;
		}

		if(stackPos >= 8) {
			parent->remainingChildren = 0;
			LogError("Warning! Render stack overflow!\n");
			continue;
		}

		u32 graphIdx = parent->remainingChildren + portalGraph.nodes[parent->graphIdx].childrenStart;
		if(graphIdx >= PortalGraph::MaxNumPortalNodes) {
			parent->remainingChildren = 0;
			if(parent->isMirror) {
				isInMirror = false;
			}
			stackPos--;
			LogError("Warning! Portal graph overflow!\n");
			continue;			
		}

		auto portalNode = &portalGraph.nodes[graphIdx];

		// New stack frame
		auto stackSlot = &portalStack[stackPos++];
		*stackSlot = {
			parent->viewProjection, graphIdx, 
			portalNode->childrenCount, false
		};

		auto ent = &scene->entities[portalNode->entityID];
		if(!ent->meshID) continue;
		if(ent->flags & Entity::FlagHidden) continue;
		if(ent->entityType != Entity::TypePortal && ent->entityType != Entity::TypeMirror) continue;

		auto targetLayer = ent->layers & portalNode->targetLayerMask;
		if(!targetLayer) continue; // This should never really happen

		vec3 ecenter = ent->position + ent->rotation * ent->centerOffset;
		vec3 dir = glm::normalize(ent->rotation * ent->planeNormal);
		vec4 plane = vec4{dir, -glm::dot(dir, ecenter)};

		stackSlot->isMirror = ent->entityType == Entity::TypeMirror;
		if(stackSlot->isMirror) {
			// https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2
			f32 a = plane.x;
			f32 b = plane.y;
			f32 c = plane.z;
			f32 d = plane.w;

			f32 ab2 = -2.f*a*b;
			f32 ac2 = -2.f*a*c;
			f32 bc2 = -2.f*b*c;
			f32 d2  = -2.f*d;

			mat4 reflection {
				1-2*a*a, ab2, ac2, 0,
				ab2, 1-2*b*b, bc2, 0,
				ac2, bc2, 1-2*c*c, 0,
				a*d2, b*d2, c*d2, 1
			};

			stackSlot->viewProjection = stackSlot->viewProjection * reflection;

			// Limit to one reflection
			if(isInMirror) continue;
			isInMirror = true;
		}

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

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

		// Write portal to stencil
		// NOTE: Polygon offset fixes z-fighting issues on some portals
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.f, -1.f);
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(parent->viewProjection));
		RenderSceneMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale);

		if(intPtl && (u32(portalNode->entityID+1u) == cam.intersectingPortalId)) {
			glDepthFunc(GL_ALWAYS);
			glBindBuffer(GL_ARRAY_BUFFER, portalClipPlaneBuffer);
			glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection));
			glUniformMatrix4fv(sh->modelLoc, 1, false, glm::value_ptr(mat4{}));
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisable(GL_CLIP_DISTANCE0);
		}

		glDisable(GL_POLYGON_OFFSET_FILL);

		// Clear Depth within stencil
		// 	- Disable color write
		// 	- Always write to depth
		// 	- Render quad at far plane
		//	- Don't write to stencil
		// 	- Reset depth write
		glDepthFunc(GL_ALWAYS);
		glDepthMask(true);

		glStencilFunc(GL_EQUAL, depthMask, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0x0);

		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(cam.projection));
		glUniformMatrix4fv(sh->modelLoc, 1, false, glm::value_ptr(mat4{}));

		glColorMask(true,true,true,true);
		// NOTE: If fog is disabled, this sets the clear color for the portal.
		//	If a colour is associated with each layer, they can effectively have their own sky colors
		glUniform4fv(sh->materialColorLoc, 1, glm::value_ptr(vec4{0,0,0,1 /*fog*/}));
		DrawQuadAtFarPlane(cam.projection); // Mixes sky with fog

		// Reset render state and prepare to render target scene
		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(stackSlot->viewProjection));
		glDepthFunc(GL_LEQUAL);
		glFrontFace(isInMirror?GL_CW:GL_CCW);

		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CULL_FACE);

		// Make sure to clip the right side
		if(glm::dot(dir, ecenter - cam.position) * (ent->entityType == Entity::TypeMirror?-1.f:1.f) < 0.f) {
			plane = -plane;
		}

		glUniform4fv(sh->clipPlaneLoc, 1, glm::value_ptr(plane));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

		for(u32 entID = 0; entID < scene->numEntities; entID++) {
			if(entID == portalNode->entityID) continue;

			auto ent = &scene->entities[entID];
			if(!ent->meshID) continue;
			if(ent->flags & Entity::FlagHidden) continue;
			if(~ent->layers & targetLayer) continue;

			if(ent->entityType == Entity::TypeTrigger) continue;

			if(ent->entityType == Entity::TypePortal
			|| ent->entityType == Entity::TypeMirror) {
				glDisable(GL_CULL_FACE);
			}else if(ent->flags & Entity::FlagDoubleSided){
				glDisable(GL_CULL_FACE);
			}else{
				glEnable(GL_CULL_FACE);
			}

			RenderSceneMesh(scene, ent->meshID, ent->position, ent->rotation, ent->scale, bool(ent->flags&Entity::FlagIgnoreFog));
		}

		glDisable(GL_CLIP_DISTANCE0);
		glFrontFace(GL_CCW);

		if(isInMirror) {
			glEnable(GL_BLEND);
			glDepthMask(false);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// NOTE: Setting alpha means mirrors mess with fog in post
			RenderSceneMeshAlpha(scene, ent->meshID, ent->position, ent->rotation, ent->scale, 0.1f);
			glDepthMask(true);
			glDisable(GL_BLEND);
		}
	}

	if(recurseGuard > PortalGraph::MaxNumPortalNodes) {
		LogError("Warning! Portal render recurse guard hit!\n");
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_DISTANCE0);
	glEnable(GL_CULL_FACE);
}
