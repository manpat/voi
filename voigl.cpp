#include "voi.h"

#include <SDL2/SDL.h>

SDL_GLContext glctx = nullptr;

void GLAPIENTRY DebugCallback(u32, u32 type, u32, u32, s32 length, const char* msg, void*) {
	if(type != GL_DEBUG_TYPE_ERROR_ARB && type != GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB) return;

	fprintf(stderr, "GLERROR: %.*s\n", length, msg);
}

static void InitGLBindings();

bool InitGL(SDL_Window* window) {
	glctx = SDL_GL_CreateContext(window);
	if(!glctx) {
		fprintf(stderr, "OpenGL context creation failed: %s\n", SDL_GetError());
		return false;
	}

	fprintf(stderr, "Version : %s\n", glGetString(GL_VERSION));
	fprintf(stderr, "GLSL    : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	fprintf(stderr, "Vendor  : %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));

	InitGLBindings();
	
	// TODO: Check to make sure that we can use all the things we are using

	const char* requiredExtensions[] {
		"GL_ARB_vertex_program",
		"GL_ARB_fragment_program",
		"GL_ARB_framebuffer_object",
		"GL_ARB_vertex_buffer_object",
	};

	bool missingRequiredExtensions = false;

	for(auto ext: requiredExtensions) {
		if(!SDL_GL_ExtensionSupported(ext)) {
			printf("Required extension missing! %s\n", ext);
			missingRequiredExtensions = true;
		}
	}

	if(missingRequiredExtensions) return false;

	// Try to enable debug output
	auto glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKARBPROC) SDL_GL_GetProcAddress("glDebugMessageCallbackARB");

	if(SDL_GL_ExtensionSupported("GL_ARB_debug_output") && glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallback((GLDEBUGPROCARB) DebugCallback, nullptr);
	}else{
		puts("Warning! Debug output not supported");
	}

	if(SDL_GL_ExtensionSupported("GL_ARB_vertex_array_object")) {
		// *Required* as of like OpenGL 3.something 
		u32 vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	SDL_GL_SetSwapInterval(0);

	return true;
}

void DeinitGL() {
	SDL_GL_DeleteContext(glctx);
}

PFNGLACTIVETEXTUREPROC glActiveTextureVoi = nullptr;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;

PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;

PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = nullptr;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation = nullptr;

PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;

PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLDRAWBUFFERSPROC glDrawBuffers = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;

PFNGLGENQUERIESPROC glGenQueries = nullptr;
PFNGLBEGINQUERYPROC glBeginQuery = nullptr;
PFNGLENDQUERYPROC glEndQuery = nullptr;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = nullptr;

static void InitGLBindings() {
	#define BINDGL(x) x = (decltype(x)) SDL_GL_GetProcAddress(#x)

	// This is available by default in linux, but windows only offers 1.1 by default, 
	//	which doesn't have this. The -Voi suffix is to avoid linking conflicts on the linux side
	glActiveTextureVoi = (decltype(glActiveTextureVoi)) SDL_GL_GetProcAddress("glActiveTexture");

	BINDGL(glGenVertexArrays);
	BINDGL(glBindVertexArray);
	BINDGL(glDisableVertexAttribArray);
	BINDGL(glEnableVertexAttribArray);
	BINDGL(glGenBuffers);
	BINDGL(glDeleteBuffers);
	BINDGL(glBindBuffer);
	BINDGL(glBufferData);
	BINDGL(glBufferSubData);
	BINDGL(glCreateShader);
	BINDGL(glShaderSource);
	BINDGL(glCompileShader);
	BINDGL(glDeleteShader);
	BINDGL(glCreateProgram);
	BINDGL(glDeleteProgram);
	BINDGL(glLinkProgram);
	BINDGL(glUseProgram);
	BINDGL(glAttachShader);
	BINDGL(glBindAttribLocation);
	BINDGL(glBindFragDataLocation);
	BINDGL(glGetShaderiv);
	BINDGL(glGetShaderInfoLog);
	BINDGL(glGetProgramiv);
	BINDGL(glGetProgramInfoLog);
	BINDGL(glUniform1i);
	BINDGL(glUniform1f);
	BINDGL(glUniform2f);
	BINDGL(glUniform3f);
	BINDGL(glUniform4f);
	BINDGL(glUniform2fv);
	BINDGL(glUniform3fv);
	BINDGL(glUniform4fv);
	BINDGL(glUniformMatrix4fv);
	BINDGL(glGetUniformLocation);
	BINDGL(glVertexAttribPointer);
	BINDGL(glGenFramebuffers);
	BINDGL(glBindFramebuffer);
	BINDGL(glDrawBuffers);
	BINDGL(glFramebufferTexture2D);
	BINDGL(glCheckFramebufferStatus);
	BINDGL(glDeleteFramebuffers);
	BINDGL(glGenQueries);
	BINDGL(glBeginQuery);
	BINDGL(glEndQuery);
	BINDGL(glGetQueryObjectuiv);
}