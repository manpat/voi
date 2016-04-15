#include "common.h"

#include "sceneloader.h"
#include "input.h"
#include "data.h"

#include <chrono>
#include <SDL2/SDL.h>

bool InitGL(SDL_Window*);
void DeinitGL();
ShaderProgram InitShaderProgram();

void InitScene(Scene*, const SceneData*);
void RenderMesh(Scene*, u16 meshID, vec3, quat);
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask);

enum {
	WindowWidth = 800,
	WindowHeight = 600
};

s32 main(s32 /*ac*/, const char** /* av*/) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		puts("SDL Init failed");
		return 1;
	}

	auto window = SDL_CreateWindow("Voi", 
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

	f32 fov = M_PI/3.f;
	f32 aspect = (f32) WindowWidth / WindowHeight;
	f32 nearDist = 0.1f;
	f32 farDist = 100.f;

	vec2 mouseRot {0,0};
	Camera camera;
	camera.position = {0,0,5};
	camera.rotation = {};
	camera.projection = glm::perspective(fov, aspect, nearDist, farDist);

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

		mouseRot += Input::GetMouseDelta();
		mouseRot.y = glm::clamp<f32>(mouseRot.y, -PI/2.f, PI/2.f);

		camera.rotation = glm::angleAxis(-mouseRot.x, vec3{0,1,0}) * glm::angleAxis(mouseRot.y, vec3{1,0,0});

		f32 speed = 0.1f;
		if(Input::GetKey(SDLK_LSHIFT)) speed *= 4.f;

		if(Input::GetKey('w')) camera.position += camera.rotation * vec3{0,0,-1} * speed;
		if(Input::GetKey('s')) camera.position += camera.rotation * vec3{0,0, 1} * speed;
		if(Input::GetKey('a')) camera.position += camera.rotation * vec3{-1,0,0} * speed;
		if(Input::GetKey('d')) camera.position += camera.rotation * vec3{ 1,0,0} * speed;

		if(Input::GetKeyDown('1')) layer = 0;
		if(Input::GetKeyDown('2')) layer = 1;
		if(Input::GetKeyDown('3')) layer = 2;
		if(Input::GetKeyDown('4')) layer = 3;
		if(Input::GetKeyDown('5')) layer = 4;
		if(Input::GetKeyDown('6')) layer = 5;
		if(Input::GetKeyDown('7')) layer = 6;
		if(Input::GetKeyDown('8')) layer = 7;
		if(Input::GetKeyDown('9')) layer = 8;

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		RenderScene(&scene, camera, 1<<layer);

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