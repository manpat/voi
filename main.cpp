#include "voi.h"

#include "sceneloader.h"
#include "input.h"

#include <chrono>
#include <SDL2/SDL.h>

enum {
	WindowWidth = 800,
	WindowHeight = 600
	// WindowWidth = 1366,
	// WindowHeight = 768
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

			float a = sin(radians(vlifetime*180.f)) * 0.35f;
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
			depth = 1-clamp(pow(depth / 80.f, .5f), 0, 1);

			vec3 fogcolor = vec3(0.1);
			outcolor.rgb = clamp(color.rgb*depth + fogcolor*(1-depth), 0, 1);
			outcolor.rgb += particle.rgb * particle.a;
			outcolor.a = 1;
		}
	)
};

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

	PhysicsContext physContext;
	if(!InitPhysics(&physContext)) {
		puts("Error! Physics init failed!");
		return 1;
	}

	Input::Init();
	Input::doCapture = true;

	SDL_WarpMouseInWindow(window, WindowWidth/2, WindowHeight/2);

	if(!InitDebugDraw()) {
		puts("Warning! Debug draw init failed");
	}

	Scene scene;
	scene.shaders[ShaderIDDefault] = CreateShaderProgram(defaultShaderSrc[0], defaultShaderSrc[1]);
	scene.shaders[ShaderIDParticles] = CreateShaderProgram(particleShaderSrc[0], particleShaderSrc[1]);
	scene.shaders[ShaderIDPost] = CreateShaderProgram(postShaderSrc[0], postShaderSrc[1]);

	// {	auto sceneData = LoadSceneData("Testing/temple.voi");
	// {	auto sceneData = LoadSceneData("Testing/portals.voi");
	{	auto sceneData = LoadSceneData("export.voi");
	// {	auto sceneData = LoadSceneData("Testing/test.voi");
	// {	auto sceneData = LoadSceneData("Testing/scaletest.voi");
		if(sceneData.numMeshes == 0 || sceneData.numEntities == 0) {
			puts("Error! Empty scene!");
			return 1;
		}

		if(!InitScene(&scene, &sceneData)) {
			puts("Error! Scene init failed!");
			return 1;
		}
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
	camera.position = {0,1.5,0};
	camera.rotation = {};
	camera.projection = glm::perspective(fov, aspect, nearDist, farDist);

	ParticleSystem particleSystem;
	if(!InitParticleSystem(&particleSystem, 1000)) {
		puts("Warning! Particle system init failed");
	}

	// Simulate 10s of dust
	for(u32 i = 0; i < 60*10; i++){
		EmitParticles(&particleSystem, 1, glm::linearRand(4.f, 20.f), camera.position);
		UpdateParticleSystem(&particleSystem, 1.f/60.f);
	}

	f32 particleEmitAccum = 0.f;

	Framebuffer fb = CreateFramebuffer(WindowWidth, WindowHeight);
	if(!fb.valid) {
		puts("Error! Framebuffer creation failed!");
		return 1;
	}

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
		EmitParticles(&particleSystem, numParticlesEmit, glm::linearRand(4.f, 20.f), camera.position);
		UpdateParticleSystem(&particleSystem, dt);

		UpdatePhysics(&physContext, &scene, dt);

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
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetDepthStencil]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetColor]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetGeneral0]);

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
			std::snprintf(titleBuffer, 256, "Voi   |   %.ffps   |   %.2fms   |   %u primitives", fps, renderdt*1000.f, primCount);
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

	// *Required* as of like OpenGL 3.something 
	u32 vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	return true;
}

void DeinitGL() {
	SDL_GL_DeleteContext(glctx);
}