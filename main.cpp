#include "common.h"

#include "sceneloader.h"
#include "input.h"
#include "data.h"

#include <SDL2/SDL.h>

bool InitGL(SDL_Window*);
void DeinitGL();
ShaderProgram InitShaderProgram();

void InitScene(Scene*, const SceneData*);
// void RenderScene(Scene*, u8 = 0);
void RenderMesh(Scene*, u16 meshID, vec3, quat);

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
	Input::doCapture = true;

	SDL_WarpMouseInWindow(window, WindowWidth/2, WindowHeight/2);

	Scene scene;
	scene.shaders[0] = InitShaderProgram();
	glUseProgram(scene.shaders[0].program);

	// {	auto sceneData = LoadSceneData("Testing/temple.voi");
	{	auto sceneData = LoadSceneData("export.voi");
		assert(sceneData.numMeshes > 0);

		InitScene(&scene, &sceneData);
		FreeSceneData(&sceneData);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);
	glClearColor(.1f, .1f, .1f, 1);
	glEnableVertexAttribArray(0);

	vec3 cameraPosition {0, 0, 5};
	vec2 cameraRot {0,0};

	f32 fov = M_PI/3.f;
	f32 aspect = (f32) WindowWidth / WindowHeight;
	f32 nearDist = 0.001f;
	f32 farDist = 100.f;

	mat4 projectionMatrix = glm::perspective(fov, aspect, nearDist, farDist);

	u32 farPlaneBuffer = 0;
	{	mat4 invProj = glm::inverse(projectionMatrix);
		vec3 p0 = vec3{invProj * vec4{-1,-1,-1, 1} * (farDist-1.f)};
		vec3 p1 = vec3{invProj * vec4{ 1,-1,-1, 1} * (farDist-1.f)};
		vec3 p2 = vec3{invProj * vec4{ 1, 1,-1, 1} * (farDist-1.f)};
		vec3 p3 = vec3{invProj * vec4{-1, 1,-1, 1} * (farDist-1.f)};

		vec3 points[] = {
			p0, p1, p2,
			p0, p2, p3,
		};

		glGenBuffers(1, &farPlaneBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, farPlaneBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	u8 layer = 0;

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

		cameraRot += Input::GetMouseDelta();
		cameraRot.y = glm::clamp<f32>(cameraRot.y, -PI/2.f, PI/2.f);

		quat cameraRotQuat{};
		cameraRotQuat = glm::angleAxis(-cameraRot.x, vec3{0,1,0}) * glm::angleAxis(cameraRot.y, vec3{1,0,0});

		f32 speed = 0.1f;
		if(Input::GetKey(SDLK_LSHIFT)) speed *= 4.f;

		if(Input::GetKey('w')) cameraPosition += cameraRotQuat * vec3{0,0,-1} * speed;
		if(Input::GetKey('s')) cameraPosition += cameraRotQuat * vec3{0,0, 1} * speed;
		if(Input::GetKey('a')) cameraPosition += cameraRotQuat * vec3{-1,0,0} * speed;
		if(Input::GetKey('d')) cameraPosition += cameraRotQuat * vec3{ 1,0,0} * speed;

		if(Input::GetKeyDown('1')) layer = 0;
		if(Input::GetKeyDown('2')) layer = 1;
		if(Input::GetKeyDown('3')) layer = 2;
		if(Input::GetKeyDown('4')) layer = 3;
		if(Input::GetKeyDown('5')) layer = 4;
		if(Input::GetKeyDown('6')) layer = 5;
		if(Input::GetKeyDown('7')) layer = 6;
		if(Input::GetKeyDown('8')) layer = 7;
		if(Input::GetKeyDown('9')) layer = 8;

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		auto sh = &scene.shaders[0];

		mat4 viewMatrix = glm::mat4_cast(glm::inverse(cameraRotQuat)) * glm::translate<f32>(-cameraPosition);
		glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(projectionMatrix * viewMatrix));

		for(u32 entID = 0; entID < scene.numEntities; entID++) {
			auto ent = &scene.entities[entID];
			if(!ent->meshID) continue;
			if(ent->flags & Entity::FlagHidden) continue;
			if(~ent->layers & 1<<layer) continue;

			if(ent->entityType == Entity::TypePortal
			|| ent->entityType == Entity::TypeMirror) {
				glDisable(GL_CULL_FACE);
			}else{
				glEnable(GL_CULL_FACE);
			}

			RenderMesh(&scene, ent->meshID, ent->position, ent->rotation);
		}

		glEnable(GL_STENCIL_TEST);

		// TODO: Create portal visibility graph and use that instead
		for(u32 entID = 0; entID < scene.numEntities; entID++) {
			auto ent = &scene.entities[entID];
			if(!ent->meshID) continue;
			if(ent->entityType != Entity::TypePortal) continue;
			if(ent->flags & Entity::FlagHidden) continue;
			if(~ent->layers & 1<<layer) continue;
			// TODO: Check portal is actually visible (mostly)

			auto targetLayer = ent->layers & ~(1<<layer);

			glDisable(GL_CLIP_DISTANCE0);
			glDisable(GL_CULL_FACE);

			// Write portal to stencil
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xff);
			glClear(GL_STENCIL_BUFFER_BIT);

			RenderMesh(&scene, ent->meshID, ent->position, ent->rotation);

			// Clear Depth within stencil
			// 	- Disable color write
			// 	- Always write to depth
			// 	- Render quad at far plane
			// 	- Reset depth write
			glDepthFunc(GL_ALWAYS);
			glStencilFunc(GL_EQUAL, 1, 0xFF);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilMask(0x0);

			glBindBuffer(GL_ARRAY_BUFFER, farPlaneBuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
			
			glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(projectionMatrix));
			glUniformMatrix4fv(sh->modelLoc, 1, false, glm::value_ptr(mat4{}));
			glUniform3fv(sh->materialColorLoc, 1, glm::value_ptr(vec3{.1f, .1f, .1f})); ////////////// Clear color

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Reset render state
			glUniformMatrix4fv(sh->viewProjectionLoc, 1, false, glm::value_ptr(projectionMatrix * viewMatrix));
			glDepthFunc(GL_LEQUAL);

			glEnable(GL_CLIP_DISTANCE0);			
			glEnable(GL_CULL_FACE);
			vec3 normal = vec3{0,1,0}; // TODO: Get from mesh
			vec3 dir = glm::normalize(normal * ent->rotation);
			glUniform4fv(sh->clipPlaneLoc, 1, glm::value_ptr(vec4{dir, -glm::dot(dir, ent->position)}));

			for(u32 entID2 = 0; entID2 < scene.numEntities; entID2++) {
				if(entID2 == entID) continue;

				auto ent = &scene.entities[entID2];
				if(!ent->meshID) continue;
				if(ent->flags & Entity::FlagHidden) continue;
				if(~ent->layers & targetLayer) continue;

				if(ent->entityType == Entity::TypePortal
				|| ent->entityType == Entity::TypeMirror) {
					glDisable(GL_CULL_FACE);
				}else{
					glEnable(GL_CULL_FACE);
				}

				RenderMesh(&scene, ent->meshID, ent->position, ent->rotation);
			}
		}

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CLIP_DISTANCE0);

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

ShaderProgram InitShaderProgram() {
	ShaderProgram ret{};

	const char* vsrc = SHADER(
		in vec3 vertex;
		out float gl_ClipDistance[1];

		uniform mat4 viewProjection;
		uniform mat4 model;

		uniform vec4 clipPlane;

		void main() {
			gl_Position = viewProjection * model * vec4(vertex, 1);
			gl_ClipDistance[0] = dot(model * vec4(vertex, 1), clipPlane);
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
		return ret;
	}

	ret.program = glCreateProgram();
	glAttachShader(ret.program, vsh);
	glAttachShader(ret.program, fsh);
	glLinkProgram(ret.program);

	s32 linkStatus;
	glGetProgramiv(ret.program, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		s32 logLength = 0;
		glGetProgramiv(ret.program, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetProgramInfoLog(ret.program, logLength, nullptr, infoLog);

		fprintf(stderr, "%s\n", infoLog);
		delete[] infoLog;

		glDeleteProgram(ret.program);
		ret.program = 0;
	}else{
		ret.viewProjectionLoc = glGetUniformLocation(ret.program, "viewProjection");
		ret.modelLoc = glGetUniformLocation(ret.program, "model");

		ret.clipPlaneLoc = glGetUniformLocation(ret.program, "clipPlane");
		ret.materialColorLoc = glGetUniformLocation(ret.program, "materialColor");
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	return ret;
}
