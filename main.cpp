#include "common.h"

#include "sceneloader.h"
#include "input.h"
#include "data.h"

#include "debugdraw.h"

#include <chrono>
#include <SDL2/SDL.h>

bool InitGL(SDL_Window*);
void DeinitGL();
ShaderProgram InitShaderProgram(const char*, const char*);

void InitScene(Scene*, const SceneData*);
void RenderMesh(Scene*, u16 meshID, vec3, quat);
void RenderScene(Scene* scene, const Camera& cam, u32 layerMask);

enum {
	TargetDepthStencil,
	TargetColor,
	TargetGeneral0,
	TargetGeneral1,
	TargetCount
};

struct Framebuffer {
	u32 fbo;
	u32 targets[TargetCount];
};

Framebuffer InitFramebuffer(u32, u32);
void DrawFullscreenQuad();

struct ParticleSystem {
	u32 numParticles;
	u32 freeIndex;
	vec3* positions;
	vec3* velocities;
	vec3* accelerations;
	f32* lifetimes;
	f32* lifeRates;

	u32 vertexBuffer;
};

void InitParticleSystem(ParticleSystem*, u32);
void UpdateParticleSystem(ParticleSystem*, f32);
void RenderParticleSystem(ParticleSystem*);
void EmitParticles(ParticleSystem*, u32, f32, vec3);

enum {
	WindowWidth = 800,
	WindowHeight = 600
};

#define SHADER(x) "#version 130\n" #x
const char* defaultShaderSrc[] = {
	SHADER(
		in vec3 vertex;
		out float gl_ClipDistance[1];

		uniform mat4 viewProjection;
		uniform mat4 model;

		uniform vec4 clipPlane;

		void main() {
			gl_Position = viewProjection * model * vec4(vertex, 1);
			gl_ClipDistance[0] = dot(model * vec4(vertex, 1), clipPlane);
		}
	),
	SHADER(
		uniform vec3 materialColor;
		out vec4 outcolor;
		out vec4 outgeneral0;

		void main() {
			outcolor = vec4(materialColor, 1);
			outgeneral0 = vec4(0);
		}
	)
};

const char* particleShaderSrc[] = {
	SHADER(
		in vec3 vertex;
		in float lifetime;
		out float vlifetime;

		uniform mat4 viewProjection;

		void main() {
			gl_Position = viewProjection * vec4(vertex, 1);
			gl_PointSize = 50.f/gl_Position.z;
			vlifetime = lifetime;
		}
	),
	SHADER(
		uniform vec3 materialColor;
		in float vlifetime;

		out vec4 outcolor;
		out vec4 outgeneral0;

		void main() {
			if(vlifetime <= 0.f) discard;

			float a = sin(radians(vlifetime*180.f)) * 0.7f;
			outgeneral0 = vec4(materialColor*a, a);
			outcolor = vec4(0);
		}
	)
};

const char* postShaderSrc[] = {
	SHADER(
		in vec2 vertex;
		out vec2 uv;

		void main() {
			gl_Position = vec4(vertex, 0, 1);
			uv = vertex * 0.5f + 0.5f;
		}
	),
	SHADER(
		uniform sampler2D depthTex;
		uniform sampler2D colorTex;
		uniform sampler2D general0Tex;
		in vec2 uv;

		out vec4 outcolor;

		void main() {
			vec4 color = texture2D(colorTex, uv);
			vec4 particle = texture2D(general0Tex, uv);
			float depth = texture2D(depthTex, uv).r * 2.f - 1.f;
			float zNear = 0.1f;
			float zFar = 1000.f;

			depth = 2.0 * zNear * zFar / (zFar + zNear - depth * (zFar - zNear));
			depth = 1-clamp(pow(depth / 40.f, .5f), 0, 1);

			vec3 fogcolor = vec3(0.1);
			outcolor.rgb = clamp(color.rgb*depth + fogcolor*(1-depth), 0, 1);
			outcolor.rgb += particle.rgb * particle.a;
			outcolor.a = 1;
		}
	)
};

// NOTE: There are better implementations to be had
f32 randf() {
	return (rand()%20000)/10000.f - 1.f;
}

extern bool debugDrawEnabled;

s32 main(s32 /*ac*/, const char** /* av*/) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		puts("SDL Init failed");
		return 1;
	}

	auto window = SDL_CreateWindow("Voi", 
		// SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_CENTERED_DISPLAY(1), SDL_WINDOWPOS_UNDEFINED, 
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

	InitDebugDraw();

	Scene scene;
	scene.shaders[ShaderIDDefault] = InitShaderProgram(defaultShaderSrc[0], defaultShaderSrc[1]);
	scene.shaders[ShaderIDParticles] = InitShaderProgram(particleShaderSrc[0], particleShaderSrc[1]);
	scene.shaders[ShaderIDPost] = InitShaderProgram(postShaderSrc[0], postShaderSrc[1]);

	// {	auto sceneData = LoadSceneData("Testing/temple.voi");
	// {	auto sceneData = LoadSceneData("Testing/portals.voi");
	{	auto sceneData = LoadSceneData("export.voi");
		assert(sceneData.numMeshes > 0);

		InitScene(&scene, &sceneData);
		FreeSceneData(&sceneData);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0, 0, 0, 0);
	glEnableVertexAttribArray(0);

	f32 fov = M_PI/3.f;
	f32 aspect = (f32) WindowWidth / WindowHeight;
	f32 nearDist = 0.1f;
	f32 farDist = 1000.f;

	vec2 mouseRot {0,0};
	u8 layer = 0;
	f32 dt = 1.f/60.f;
	f32 fpsTimeAccum = 0.f;
	u32 fpsFrameCount = 0;

	auto beginTime = std::chrono::high_resolution_clock::now();

	u32 primCountQuery;
	glGenQueries(1, &primCountQuery);

	Camera camera;
	camera.position = {0,0,5};
	camera.rotation = {};
	camera.projection = glm::perspective(fov, aspect, nearDist, farDist);

	ParticleSystem particleSystem;
	InitParticleSystem(&particleSystem, 1000);

	f32 particleEmitAccum = 0.f;

	Framebuffer fb = InitFramebuffer(WindowWidth, WindowHeight);

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

		vec3 vel {};
		if(Input::GetMapped(Input::Forward))	vel += camera.rotation * vec3{0,0,-1};
		if(Input::GetMapped(Input::Backward))	vel += camera.rotation * vec3{0,0, 1};
		if(Input::GetMapped(Input::Left))		vel += camera.rotation * vec3{-1,0,0};
		if(Input::GetMapped(Input::Right))		vel += camera.rotation * vec3{ 1,0,0};

		if(glm::length(vel) > 1.f){
			vel = glm::normalize(vel);
		}

		f32 speed = 5.f;
		if(Input::GetKey(SDLK_LSHIFT)) speed *= 4.f;
		vel *= speed;

		camera.position += vel * dt;

		auto viewProjection = camera.projection * glm::mat4_cast(
			glm::inverse(camera.rotation)) * glm::translate<f32>(-camera.position);

		if(Input::GetKeyDown('1')) layer = 0;
		if(Input::GetKeyDown('2')) layer = 1;
		if(Input::GetKeyDown('3')) layer = 2;
		if(Input::GetKeyDown('4')) layer = 3;
		if(Input::GetKeyDown('5')) layer = 4;
		if(Input::GetKeyDown('6')) layer = 5;
		if(Input::GetKeyDown('7')) layer = 6;
		if(Input::GetKeyDown('8')) layer = 7;
		if(Input::GetKeyDown('9')) layer = 8;

		if(Input::GetKeyDown('c')) Input::doCapture ^= true;
		if(Input::GetKeyDown(SDLK_F1)) debugDrawEnabled ^= true;

		particleEmitAccum += (glm::length(vel)*0.8f + 50.f) * dt;

		u32 numParticlesEmit = (u32) particleEmitAccum;
		particleEmitAccum -= numParticlesEmit;
		EmitParticles(&particleSystem, numParticlesEmit, 7.f + randf() * 3.f, camera.position);
		UpdateParticleSystem(&particleSystem, dt);

		auto beginRenderTime = std::chrono::high_resolution_clock::now();
		glBeginQuery(GL_PRIMITIVES_GENERATED, primCountQuery);
		glUseProgram(scene.shaders[ShaderIDDefault].program);

		glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			RenderScene(&scene, camera, 1<<layer);

			glUseProgram(scene.shaders[ShaderIDParticles].program);
			glUniform3fv(scene.shaders[ShaderIDParticles].materialColorLoc, 1, glm::value_ptr(vec3{.5}));
			glUniformMatrix4fv(scene.shaders[ShaderIDParticles].viewProjectionLoc, 1, false, 
				glm::value_ptr(viewProjection));

			glEnable(GL_PROGRAM_POINT_SIZE);
			glDepthMask(false);
			RenderParticleSystem(&particleSystem);
			glDepthMask(true);
			glDisable(GL_PROGRAM_POINT_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glUseProgram(scene.shaders[ShaderIDPost].program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb.targets[TargetDepthStencil]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fb.targets[TargetColor]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fb.targets[TargetGeneral0]);

		glUniform1i(scene.shaders[ShaderIDPost].depthTexLoc, 0);
		glUniform1i(scene.shaders[ShaderIDPost].colorTexLoc, 1);
		glUniform1i(scene.shaders[ShaderIDPost].general0TexLoc, 2);

		DrawFullscreenQuad();

		glBindTexture(GL_TEXTURE_2D, 0);

		DrawDebug(viewProjection);

		auto endRenderTime = std::chrono::high_resolution_clock::now();
		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
		glEndQuery(GL_PRIMITIVES_GENERATED);

		auto endTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::duration<f32>>(endTime-beginTime).count();
		f32 renderdt = std::chrono::duration_cast<std::chrono::duration<f32>>(endRenderTime-beginRenderTime).count();
		beginTime = endTime;
		fpsTimeAccum += dt;
		fpsFrameCount++;

		if(fpsTimeAccum > 0.3f) {
			f32 fps = fpsFrameCount/fpsTimeAccum;
			fpsFrameCount = 0;
			fpsTimeAccum = 0;

			u32 primCount = 0;
			glGetQueryObjectuiv(primCountQuery, GL_QUERY_RESULT, &primCount);

			char titleBuffer[256];
			std::snprintf(titleBuffer, 256, "Voi   |   %.ffps   |   %.2fms   |   %.4u primitives", fps, renderdt*1000.f, primCount);
			SDL_SetWindowTitle(window, titleBuffer);
		}

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

ShaderProgram InitShaderProgram(const char* vsrc, const char* fsrc) {
	ShaderProgram ret{};

	u32 vsh = CreateShader(vsrc, GL_VERTEX_SHADER);
	u32 fsh = CreateShader(fsrc, GL_FRAGMENT_SHADER);

	if(!vsh || !fsh) {
		fprintf(stderr, "Shader compilation failed\n");
		return ret;
	}

	ret.program = glCreateProgram();
	glAttachShader(ret.program, vsh);
	glAttachShader(ret.program, fsh);

	glBindAttribLocation(ret.program, 0, "vertex");

	glBindFragDataLocation(ret.program, 0, "outcolor");
	glBindFragDataLocation(ret.program, 1, "outgeneral0");
	glBindFragDataLocation(ret.program, 2, "outgeneral1");

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

		ret.materialColorLoc = glGetUniformLocation(ret.program, "materialColor");
		ret.clipPlaneLoc = glGetUniformLocation(ret.program, "clipPlane");

		ret.depthTexLoc = glGetUniformLocation(ret.program, "depthTex");
		ret.colorTexLoc = glGetUniformLocation(ret.program, "colorTex");
		ret.general0TexLoc = glGetUniformLocation(ret.program, "general0Tex");
		ret.general1TexLoc = glGetUniformLocation(ret.program, "general1Tex");
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	return ret;
}

void InitParticleSystem(ParticleSystem* sys, u32 numParticles) {
	sys->freeIndex = 0;
	sys->numParticles = numParticles;
	sys->positions = new vec3[numParticles];
	sys->velocities = new vec3[numParticles];
	sys->accelerations = new vec3[numParticles];
	sys->lifetimes = new f32[numParticles];
	sys->lifeRates = new f32[numParticles];

	glGenBuffers(1, &sys->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sys->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numParticles * sizeof(f32) * 4, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UpdateParticleSystem(ParticleSystem* sys, f32 dt) {
	for(u32 i = 0; i < sys->numParticles; i++) {
		if(sys->lifetimes[i] < 0.f) continue;
		sys->lifetimes[i] -= sys->lifeRates[i] * dt;

		sys->positions[i] += sys->velocities[i] * dt;
		sys->velocities[i] += sys->accelerations[i] * dt;
	}

	glBindBuffer(GL_ARRAY_BUFFER, sys->vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sys->numParticles * sizeof(vec3), sys->positions);
	glBufferSubData(GL_ARRAY_BUFFER, sys->numParticles * sizeof(vec3), sys->numParticles * sizeof(f32), sys->lifetimes);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderParticleSystem(ParticleSystem* sys) {
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, sys->vertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, (void*)((u64)sys->numParticles * sizeof(vec3)));

	glDrawArrays(GL_POINTS, 0, sys->numParticles);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(1);
}

void EmitParticles(ParticleSystem* sys, u32 count, f32 lifetime, vec3 position) {
	u32 i = sys->freeIndex;
	while(count > 0) {
		do {
			sys->positions[sys->freeIndex] = position + vec3{15*randf(), 6*randf(), 15*randf()};
			sys->velocities[sys->freeIndex] = glm::normalize(vec3{randf(),randf(),randf()})*0.05f;
			sys->accelerations[sys->freeIndex] = glm::normalize(vec3{randf(),randf()-0.5f,randf()})*0.03f;
			sys->lifetimes[sys->freeIndex] = 1.f;
			sys->lifeRates[sys->freeIndex] = 1.f/lifetime;
		} while(i++ < sys->numParticles && --count);
		
		if(i >= sys->numParticles) {
			i = 0;
		}
	}
	sys->freeIndex = i;
}

Framebuffer InitFramebuffer(u32 width, u32 height) {
	static u32 fbTargetTypes[] {GL_DEPTH24_STENCIL8, GL_RGB8, GL_RGBA8};
	static u32 fbTargetFormats[] {GL_DEPTH_STENCIL, GL_RGB, GL_RGBA};
	static u32 fbTargetAttach[] {GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	static u32 fbTargetIntType[] {GL_UNSIGNED_INT_24_8, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};

	Framebuffer fb;
	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
	glDrawBuffers(2, &fbTargetAttach[1]);

	glGenTextures(3, fb.targets);
	for(u8 i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, fb.targets[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, fbTargetTypes[i], width, height, 0, fbTargetFormats[i], fbTargetIntType[i], nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, fbTargetAttach[i], GL_TEXTURE_2D, fb.targets[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		puts("Warning! Framebuffer incomplete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
}

void DrawFullscreenQuad() {
	static u32 vbo = 0;
	if(!vbo) {
		vec2 verts[] = {
			vec2{-1,-1},
			vec2{ 1,-1},
			vec2{ 1, 1},
			vec2{-1, 1},
		};

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, nullptr);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}