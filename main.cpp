#include "common.h"
#include "sceneloader.h"

#include "input.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>

bool InitGL(SDL_Window*);
void DeinitGL();

enum {
	WindowWidth = 800,
	WindowHeight = 600
};

s32 main(s32 /*ac*/, const char**/* av*/) {
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
	Input::Init();
	Input::doCapture = false;

	glEnable(GL_DEPTH_TEST);

	LoadScene("cube.voi");

	// *Required* as of like OpenGL 3.something 
	u32 vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClearColor(1,0,1, 1);

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
