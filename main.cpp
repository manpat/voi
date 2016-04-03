#include "common.h"
#include "sceneloader.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

void APIENTRY DebugCallback(u32, u32 type, u32, u32, s32, const char* msg, void*);

enum {
	WindowWidth = 800,
	WindowHeight = 600
};

s32 main(s32 ac, const char** av) {
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

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	
	auto glctx = SDL_GL_CreateContext(window);
	if(!glctx) {
		puts("OpenGL context creation failed");
		return 1;
	}

	// Set up some initial GL state
	glEnable(GL_DEPTH_TEST);

	// Try to enable debug output
	if(SDL_GL_ExtensionSupported("GL_ARB_debug_output")){
		auto mglDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKARBPROC) 
			SDL_GL_GetProcAddress("glDebugMessageCallbackARB");

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		mglDebugMessageCallback((GLDEBUGPROCARB) DebugCallback, nullptr);
	}else{
		puts("Warning! Debug output not supported");
	}

	LoadScene("cube.voi");

	// *Required* as of like OpenGL 3.sometihng 
	u32 vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClearColor(1,0,1, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	SDL_GL_SwapWindow(window);

	SDL_Delay(150);

	SDL_GL_DeleteContext(glctx);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

void APIENTRY DebugCallback(u32, u32 type, u32, u32, s32 length, const char* msg, void*) {
	if(type != GL_DEBUG_TYPE_ERROR_ARB && type != GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB) return;

	printf("GLERROR: %.*s\n", length, msg);
}