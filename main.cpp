#include "voi.h"

#include "sceneloader.h"
#include "input.h"

#include <chrono>
#include <SDL2/SDL.h>

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

		uniform float pixelSize;
		uniform mat4 viewProjection;

		void main() {
			gl_Position = viewProjection * vec4(vertex, 1);
			gl_PointSize = pixelSize/gl_Position.z;
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
		uniform mat4 viewProjection;
		in vec2 uv;

		out vec4 outcolor;

		void main() {
			vec4 color = texture2D(colorTex, uv);
			vec4 particle = texture2D(general0Tex, uv);
			float depth = texture2D(depthTex, uv).r * 2.f - 1.f;
			float zNear = 0.1f;
			float zFar = 1000.f;

			// depth = 2.0 * zNear * zFar / (zFar + zNear - depth * (zFar - zNear));
			vec4 wp = inverse(viewProjection) * vec4(uv*2 - 1, depth*0.5 + 0.5, 1);
			depth = length(wp/wp.w); // Distance from eye


			float fogmix = 1-clamp(pow(depth / 150.f, 0.4f), 0, 1);

			// vec3 fogcolor = vec3(0.1);
			// vec3 fogcolor = vec3(1, .6, .2) * 0.1;
			vec3 fogcolor = vec3(0.25, 0.0, 0.1)*0.2;
			outcolor.rgb = clamp(color.rgb*fogmix + fogcolor*(1-fogmix), 0, 1);
			outcolor.rgb += particle.rgb * particle.a;
			outcolor.a = 1;

			// float step = 8.f;
			// outcolor.rgb = floor(outcolor.rgb*step)/step;
		}
	)
};

const char* uiShaderSrc[] = {
	SHADER(
		in vec3 vertex;
		in vec2 uv;
		out vec2 vuv;

		uniform mat4 viewProjection;

		void main() {
			gl_Position = viewProjection*vec4(vertex, 1);
			vuv = uv;
		}
	),
	SHADER(
		uniform sampler2D colorTex;
		in vec2 vuv;
		out vec4 outcolor;

		void main() {
			vec4 color = texture2D(colorTex, vuv);
			color.rgb *= color.a; // We premultiply our alphas in these parts
			outcolor = color;
		}
	)
};

extern bool debugDrawEnabled;

u32 cursorTextures[2];
u32 cursorVBO;
u32 cursorUVBO;
u8 interactiveHover = 0;

u32 windowWidth;
u32 windowHeight;

s32 main(s32 ac, const char** av) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		puts("SDL Init failed");
		return 1;
	}

	ParseCLOptions(ac, av);
	LoadOptions();

	windowWidth = GetIntOption("window.width");
	windowHeight = GetIntOption("window.height");
	bool fullscreen = GetBoolOption("window.fullscreen");

	auto window = SDL_CreateWindow("Voi", 
		SDL_WINDOWPOS_CENTERED_DISPLAY(1), SDL_WINDOWPOS_UNDEFINED, 
		windowWidth, windowHeight, SDL_WINDOW_OPENGL);

	if(!window) {
		puts("Window creation failed");
		return 1;
	}

	if(!InitGL(window)) return 1;

	Input::Init();
	Input::doCapture = true;

	SDL_WarpMouseInWindow(window, windowWidth/2, windowHeight/2);
	SDL_ShowCursor(0);

	if(!InitDebugDraw()) {
		puts("Warning! Debug draw init failed");
	}

	f32 fov = glm::radians<f32>(GetFloatOption("graphics.fov"));
	f32 aspect = (f32) windowWidth / windowHeight;
	f32 nearDist = 0.01f;
	f32 farDist = 1000.f;

	Camera camera;
	camera.projection = glm::perspective(fov, aspect, nearDist, farDist);

	auto playerEntity = AllocateEntity();
	playerEntity->layers = 1<<0;
	playerEntity->position = {0,1,0};
	playerEntity->rotation = quat{};
	playerEntity->scale = {1,1,1};
	playerEntity->name = strdup("Player");
	playerEntity->nameLength = strlen(playerEntity->name);
	playerEntity->entityType = Entity::TypePlayer;
	playerEntity->colliderType = ColliderCapsule;
	playerEntity->extents = vec3{2.f, 3.f, 2.f}/2.f;
	playerEntity->player.eyeOffset = vec3{0, 1.2f, 0};
	playerEntity->player.camera = &camera;
	printf("Player id: %u\n", playerEntity->id);

	Scene scene;
	scene.shaders[ShaderIDDefault] = CreateShaderProgram(defaultShaderSrc[0], defaultShaderSrc[1]);
	scene.shaders[ShaderIDParticles] = CreateShaderProgram(particleShaderSrc[0], particleShaderSrc[1]);
	scene.shaders[ShaderIDPost] = CreateShaderProgram(postShaderSrc[0], postShaderSrc[1]);
	scene.shaders[ShaderIDUI] = CreateShaderProgram(uiShaderSrc[0], uiShaderSrc[1]);

	if(!InitPhysics(&scene.physicsContext)) {
		puts("Error! Physics init failed!");
		return 1;
	}

	// {	auto sceneData = LoadSceneData("Testing/temple.voi");
	// {	auto sceneData = LoadSceneData("Testing/portals.voi");
	// {	auto sceneData = LoadSceneData("export.voi");
	{	auto sceneData = LoadSceneData("Testing/test.voi");
	// {	auto sceneData = LoadSceneData("Testing/scaletest.voi");
		if(sceneData.numMeshes == 0 || sceneData.numEntities == 0) {
			puts("Error! Empty scene!");
			return 1;
		}

		if(!InitScene(&scene, &sceneData)) {
			puts("Error! Scene init failed!");
			return 1;
		}

		// TODO: Fuck this off. I'm just testing the crosshair
		for(u32 i = 0; i < scene.numEntities; i++) {
			if(scene.entities[i].colliderType == ColliderCube
				&& scene.entities[i].entityType == Entity::TypeGeometry){
				printf("Entity [%u] %.*s was made interactive\n", i, 
					(u32)scene.entities[i].nameLength, scene.entities[i].name);
				scene.entities[i].flags |= Entity::FlagInteractive;
			}	
		}

		playerEntity->scene = &scene;
		if(!InitEntityPhysics(playerEntity, nullptr)) {
			puts("Error! Entity physics init failed for player!");
			return 1;
		}

		InitEntity(playerEntity);

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

	f32 dt = 1.f/60.f;
	f32 fpsTimeAccum = 0.f;
	u32 fpsFrameCount = 0;

	auto beginTime = std::chrono::high_resolution_clock::now();

	u32 primCountQuery;
	glGenQueries(1, &primCountQuery);

	ParticleSystem particleSystem;
	if(!InitParticleSystem(&particleSystem, 10000)) {
		puts("Warning! Particle system init failed");
	}

	// Simulate 3s of dust
	for(u32 i = 0; i < 60*3; i++){
		EmitParticles(&particleSystem, 1, glm::linearRand(4.f, 20.f), playerEntity->position);
		UpdateParticleSystem(&particleSystem, 1.f/60.f);
	}

	f32 particleEmitAccum = 0.f;

	f32 multisampleLevel = 1.;
	bool filter = true;

	if(!strcmp(GetStringOption("graphics.multisample"), "indiepls")) { // NOTE: Sshhhh...
		multisampleLevel = 0.08;
		filter = false;
	}else if(GetBoolOption("graphics.multisample")){
		multisampleLevel = 2.;
	}

	// TODO: Better antialiasing method
	Framebuffer fb = CreateFramebuffer(windowWidth*multisampleLevel, windowHeight*multisampleLevel, filter);
	if(!fb.valid) {
		puts("Error! Framebuffer creation failed!");
		return 1;
	}

	auto SetFullscreen = [window, &fb, multisampleLevel, filter, &camera, fov, nearDist, farDist, &aspect] (bool fullscreen) {
		// TODO: Choose between native and fake fullscreen based on resolution?
		//	or have an option?
		if(SDL_SetWindowFullscreen(window, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: 0) < 0) 
			return false;

		if(fullscreen) {
			SDL_GetWindowSize(window, (s32*)&windowWidth, (s32*)&windowHeight);
		}else{
			windowWidth = GetIntOption("window.width");
			windowHeight = GetIntOption("window.height");
			// NOTE: Should be fine given that we start windowed
			// SDL_SetWindowSize(window, windowWidth, windowHeight);
		}

		if(Input::doCapture) {
			SDL_WarpMouseInWindow(window, windowWidth/2, windowHeight/2);
		}

		if(fb.valid) DestroyFramebuffer(&fb);

		fb = CreateFramebuffer(windowWidth*multisampleLevel, windowHeight*multisampleLevel, filter);
		if(!fb.valid) {
			puts("Error! Framebuffer creation failed!");
			return false;
		}

		aspect = (f32) windowWidth / windowHeight;
		camera.projection = glm::perspective(fov, aspect, nearDist, farDist);

		return true;
	};

	if(!SetFullscreen(fullscreen)) return 1;

	SDL_Event e;
	bool running = true;
	while(running) {
		auto beginFrameTime = std::chrono::high_resolution_clock::now();
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

		if(Input::GetKeyDown(SDLK_F11)) {
			SetFullscreen(fullscreen ^= true);
		}

		vec3 vel = GetEntityVelocity(playerEntity);
		particleEmitAccum += (glm::length(vel)*10.f + 30.f) * dt;

		u32 numParticlesEmit = (u32) particleEmitAccum;
		particleEmitAccum -= numParticlesEmit;
		EmitParticles(&particleSystem, numParticlesEmit, glm::linearRand(4.f, 20.f), playerEntity->position + vel*2.f);
		UpdateParticleSystem(&particleSystem, dt);

		UpdateAllEntities(dt);

		camera.rotation = playerEntity->rotation * glm::angleAxis(playerEntity->player.mouseRot.y, vec3{1,0,0});
		// NOTE: UpdatePhysics fucks with player rotation for some reason
		UpdatePhysics(&scene, dt);

		// Update camera position *after* physics have been taken into account
		camera.position = playerEntity->position + playerEntity->player.eyeOffset;
		auto viewProjection = camera.projection * glm::mat4_cast(
			glm::inverse(camera.rotation)) * glm::translate<f32>(-camera.position);

		glBeginQuery(GL_PRIMITIVES_GENERATED, primCountQuery);

		// Draw scene into framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
			glViewport(0,0, fb.width, fb.height);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			glUseProgram(scene.shaders[ShaderIDDefault].program);
			RenderScene(&scene, camera, playerEntity->layers);

			glUseProgram(scene.shaders[ShaderIDParticles].program);
			glUniform3fv(scene.shaders[ShaderIDParticles].materialColorLoc, 1, glm::value_ptr(vec3{.5}));
			glUniformMatrix4fv(scene.shaders[ShaderIDParticles].viewProjectionLoc, 1, false, 
				glm::value_ptr(viewProjection));

			// HACK: Because we're using point primitives for particles and they use pixels,
			// 	we have to adjust particle size between resolutions
			glUniform1f(glGetUniformLocation(scene.shaders[ShaderIDParticles].program, "pixelSize"), 
				fb.height*50.f/600.f);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glDepthMask(false);
			RenderParticleSystem(&particleSystem);
			glDepthMask(true);
			glDisable(GL_PROGRAM_POINT_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0,0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(scene.shaders[ShaderIDPost].program);

		// Draw scene with post effects
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetDepthStencil]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetColor]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetGeneral0]);

		glUniform1i(scene.shaders[ShaderIDPost].depthTexLoc, 0);
		glUniform1i(scene.shaders[ShaderIDPost].colorTexLoc, 1);
		glUniform1i(scene.shaders[ShaderIDPost].general0TexLoc, 2);
		glUniformMatrix4fv(scene.shaders[ShaderIDPost].viewProjectionLoc, 1, false,
			glm::value_ptr(camera.projection));

		DrawFullscreenQuad();

		// Draw cursor
		glUseProgram(scene.shaders[ShaderIDUI].program);
		glEnableVertexAttribArray(1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cursorTextures[interactiveHover&1]);

		f32 a = aspect;
		mat4 uiProj{
			1,0,0,0,
			0,a,0,0,
			0,0,1,0,
			0,0,0,1,
		};

		glUniform1i(scene.shaders[ShaderIDUI].colorTexLoc, 0);
		glUniformMatrix4fv(scene.shaders[ShaderIDUI].viewProjectionLoc, 1,
			false, glm::value_ptr(uiProj));

		glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, cursorUVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, nullptr);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisableVertexAttribArray(1);
		glEnable(GL_DEPTH_TEST);
		DrawDebug(viewProjection);

		auto endFrameTime = std::chrono::high_resolution_clock::now();
		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
		glEndQuery(GL_PRIMITIVES_GENERATED);

		auto endTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::duration<f32>>(endTime-beginTime).count();
		f32 renderdt = std::chrono::duration_cast<std::chrono::duration<f32>>(endFrameTime-beginFrameTime).count();
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

	DeinitParticleSystem(&particleSystem);

	DeinitEntityPhysics(playerEntity);
	FreeEntity(playerEntity);

	DeinitScene(&scene);
	DestroyFramebuffer(&fb);

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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

	// TODO: Check to make sure that we can use all the things we are using
	// TODO: Possibly fuck glew off and get fptrs ourselves, or at least cut down glew
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

	cursorTextures[0] = LoadTexture("GameData/UI/cursor.png");
	cursorTextures[1] = LoadTexture("GameData/UI/cursor2.png");

	f32 s = GetFloatOption("graphics.cursorsize")/100.f;
	vec3 verts[] = {
		vec3{-s,-s, 0},
		vec3{ s,-s, 0},
		vec3{ s, s, 0},
		vec3{-s, s, 0},
	};

	vec2 uvs[] = {
		vec2{0,1},
		vec2{1,1},
		vec2{1,0},
		vec2{0,0},
	};

	glGenBuffers(1, &cursorVBO);
	glGenBuffers(1, &cursorUVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, cursorUVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void DeinitGL() {
	glDeleteTextures(2, cursorTextures);

	SDL_GL_DeleteContext(glctx);
}