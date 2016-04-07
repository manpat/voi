#include "common.h"
#include "sceneloader.h"

#include "input.h"
#include "data.h"

#define GLEW_STATIC
#include "glew.h"
#include <SDL2/SDL.h>

bool InitGL(SDL_Window*);
void DeinitGL();
u32 InitShaderProgram();

enum {
	WindowWidth = 800,
	WindowHeight = 600
};

s32 main(s32 /*ac*/, const char** /* av*/) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		puts("SDL Init failed");
		return 1;
	}

	auto window = SDL_CreateWindow("OpenGL", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		WindowWidth, WindowHeight, SDL_WINDOW_OPENGL);

	if(!window) {
		puts("Window creation failed");
		return 1;
	}

	if(!InitGL(window)) return 1;

	// *Required* as of like OpenGL 3.something 
	u32 vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	Input::Init();
	Input::doCapture = false;

	u32 program = InitShaderProgram();
	glUseProgram(program);

	Scene scene;
	auto sceneData = LoadSceneData("export.voi");
	assert(sceneData.numMeshes > 0);

	// THIS IS WAY OVERKILL!!
	// It would be better to actually figure out how long the names are
	scene.nameArenaSize = (sceneData.numMaterials + sceneData.numEntities) * 256u;
	scene.nameArena = new char[scene.nameArenaSize];
	scene.nameArenaFree = scene.nameArena;
	std::memset(scene.nameArena, 0, scene.nameArenaSize);

	scene.numEntities = sceneData.numEntities;
	scene.entities = new Entity[scene.numEntities];

	scene.numMeshes = sceneData.numMeshes;
	scene.meshes = new Mesh[scene.numMeshes];

	for(u32 meshID = 0; meshID < scene.numMeshes; meshID++) {
		auto meshData = &sceneData.meshes[meshID];
		auto mesh = &scene.meshes[meshID];
		mesh->numTriangles = meshData->numTriangles;
		mesh->elementType = GL_UNSIGNED_INT;
		if(meshData->numVertices < 256) {
			mesh->elementType = GL_UNSIGNED_BYTE;
		}else if(meshData->numVertices < 65536) {
			mesh->elementType = GL_UNSIGNED_SHORT;
		}

		// Figure out submeshes
		{	mesh->numSubmeshes = 0;
			u8 prevMatID = 0;
			for(u32 i = 0; i < mesh->numTriangles; i++) {
				u8 mat = meshData->materialIDs[i];
				if(mat > prevMatID) {
					mesh->numSubmeshes++;
					prevMatID = mat;
				}
			}
		
			auto submeshes = &mesh->submeshesInline[0];
			if(mesh->numSubmeshes > Mesh::MaxInlineSubmeshes) {
				mesh->submeshes = submeshes = new Mesh::Submesh[mesh->numSubmeshes];
			}
		
			prevMatID = 0;
			u32 matStart = 0;
		
			for(u32 i = 0; i < mesh->numTriangles; i++) {
				u8 mat = meshData->materialIDs[i];
				if(mat > prevMatID) {
					if(prevMatID != 0) {
						*submeshes++ = {matStart, prevMatID};
					}
		
					matStart = i;
					prevMatID = mat;
				}
			}
		
			*submeshes++ = {matStart, prevMatID};
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
	std::memset(scene.materials, 0, sizeof(scene.materials));
	for(u16 i = 0; i < sceneData.numMaterials; i++){
		auto to = &scene.materials[i];
		auto from = &sceneData.materials[i];

		std::memcpy(scene.nameArenaFree, from->name, from->nameLength);
		to->name = scene.nameArenaFree;
		scene.nameArenaFree += from->nameLength;
		*scene.nameArenaFree++ = '\0';

		to->color = from->color;
		// TODO: Shader stuff when added
	}

	// Do entity stuff
	std::memset(scene.entities, 0, scene.numEntities * sizeof(Entity));
	for(u32 i = 0; i < sceneData.numEntities; i++) {
		auto to = &scene.entities[i];
		auto from = &sceneData.entities[i];

		std::memcpy(scene.nameArenaFree, from->name, from->nameLength);
		to->nameLength = from->nameLength;
		to->name = scene.nameArenaFree;
		scene.nameArenaFree += to->nameLength;
		*scene.nameArenaFree++ = '\0';

		to->id = i+1;
		to->flags = 0; // TODO

		to->layer = from->layer;
		to->position = from->position;
		to->rotation = quat(from->rotation);

		to->parentID = from->parentID;
		to->meshID = from->meshID;
		to->entityType = from->entityType;
		to->colliderType = from->colliderType;
	}

	FreeSceneData(&sceneData);

	glEnable(GL_DEPTH_TEST);
	glClearColor(.1f, .1f, .1f, 1);
	glEnableVertexAttribArray(0);

	vec3 cameraPosition {0, 0, 5};
	f32 fov = M_PI/3.f;
	f32 aspect = (f32) WindowWidth / WindowHeight;
	f32 nearDist = 0.001f;
	f32 farDist = 100.f;

	mat4 projectionMatrix = glm::perspective(fov, aspect, nearDist, farDist);
	mat4 viewMatrix = glm::translate<f32>(-cameraPosition);
	mat4 modelMatrix = mat4(1.f);

	u32 materialColorLoc = glGetUniformLocation(program, "materialColor");
	u32 viewProjectionLoc = glGetUniformLocation(program, "viewProjection");
	u32 modelLoc = glGetUniformLocation(program, "model");

	SDL_Event e;
	bool running = true;
	while(running) {
		while(SDL_PollEvent(&e)) {
			Input::InjectSDLEvent(e);
			if(e.type == SDL_QUIT) {
				running = false;
			}
		}
		Input::UpdateMouse(window);

		if(Input::GetKeyDown(SDLK_ESCAPE)) {
			running = false;
		}

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		static f32 t = 0.f;
		viewMatrix 
			= glm::translate<f32>(-cameraPosition)
			* glm::rotate<f32>(t += 0.01f, glm::normalize(vec3{0,-1,0}));

		glUniformMatrix4fv(viewProjectionLoc, 1, false, glm::value_ptr(projectionMatrix * viewMatrix));

		for(u32 entID = 0; entID < scene.numEntities; entID++) {
			auto ent = &scene.entities[entID];
			if(!ent->meshID) continue;
			auto mesh = &scene.meshes[ent->meshID-1];

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
					auto mat = &scene.materials[sms[i].materialID-1];
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
		// for(u32 meshID = 0; meshID < scene.numMeshes; meshID++) {
		// 	auto mesh = &scene.meshes[meshID];

		// 	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		// 	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
		// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

		// 	u32 end = mesh->numTriangles;
		// 	auto sms = (mesh->numSubmeshes <= Mesh::MaxInlineSubmeshes)? 
		// 		&mesh->submeshesInline[0] : mesh->submeshes;
			
		// 	// Draw in reverse order to make getting triangle count easier
		// 	for(s32 i = mesh->numSubmeshes-1; i >= 0; i--) {
		// 		if(sms[i].materialID > 0) {
		// 			auto mat = &scene.materials[sms[i].materialID-1];
		// 			glUniform3fv(materialColorLoc, 1, glm::value_ptr(mat->color));
		// 		}else{
		// 			glUniform3fv(materialColorLoc, 1, glm::value_ptr(vec3{1,0,1}));
		// 		}

		// 		u32 begin = sms[i].startIndex;
		// 		u32 count = end - begin;
		// 		end = begin;

		// 		glDrawElements(GL_TRIANGLES, count*3, mesh->elementType, 
		// 			(void*) (begin*3ull*mesh->elementSize));
		// 	}
		// }

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		SDL_GL_SwapWindow(window);
		SDL_Delay(1);

		Input::ClearFrameState();
	}

	Input::Deinit();
	DeinitGL();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

void GLAPIENTRY DebugCallback(u32, u32 type, u32, u32, s32 length, const char* msg, void*) {
	if(type != GL_DEBUG_TYPE_ERROR_ARB && type != GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB) return;

	fprintf(stderr, "GLERROR: %.*s\n", length, msg);
}

SDL_GLContext glctx = nullptr;

bool InitGL(SDL_Window* window) {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	
	glctx = SDL_GL_CreateContext(window);
	if(!glctx) {
		puts("OpenGL context creation failed");
		return false;
	}

	glewExperimental = true;
	auto glewerr = glewInit();
	if(glewerr != GLEW_OK) {
		printf("GLEW init failed: %s\n", glewGetErrorString(glewerr));
		return false;
	}

	// Try to enable debug output
	if(GLEW_ARB_debug_output){
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallbackARB((GLDEBUGPROCARB) DebugCallback, nullptr);
	}else{
		puts("Warning! Debug output not supported");
	}

	return true;
}

void DeinitGL() {
	SDL_GL_DeleteContext(glctx);
}

#define SHADER(x) "#version 130\n" #x

u32 CreateShader(const char* src, u32 type) {
	u32 id = glCreateShader(type);

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	s32 status = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if(!status) {
		s32 logLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetShaderInfoLog(id, logLength, nullptr, infoLog);

		fprintf(stderr, "%s\n", infoLog);
		delete[] infoLog;

		glDeleteShader(id);
		return 0;
	}

	return id;
}

u32 InitShaderProgram() {
	const char* vsrc = SHADER(
		in vec3 vertex;

		uniform mat4 viewProjection;
		uniform mat4 model;

		void main() {
			gl_Position = viewProjection * model * vec4(vertex, 1);
		}
	);

	const char* fsrc = SHADER(
		out vec4 outcolor;
		
		uniform vec3 materialColor;

		void main() {
			outcolor = vec4(materialColor, 1);
		}
	);

	u32 vsh = CreateShader(vsrc, GL_VERTEX_SHADER);
	u32 fsh = CreateShader(fsrc, GL_FRAGMENT_SHADER);

	if(!vsh || !fsh) {
		fprintf(stderr, "Shader compilation failed\n");
		return 0;
	}

	u32 program = glCreateProgram();
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);
	glLinkProgram(program);

	s32 linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		s32 logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetProgramInfoLog(program, logLength, nullptr, infoLog);

		fprintf(stderr, "%s\n", infoLog);
		delete[] infoLog;

		glDeleteProgram(program);
		return 0;
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	return program;
}
