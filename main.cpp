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
		flat out int faceID;

		uniform mat4 viewProjection;
		uniform mat4 model;

		uniform vec4 clipPlane;

		void main() {
			gl_Position = viewProjection * model * vec4(vertex, 1);
			gl_ClipDistance[0] = dot(model * vec4(vertex, 1), clipPlane);
			faceID = int(gl_VertexID/3);
		}
	),
	SHADER(
		uniform vec4 materialColor;
		flat in int faceID;
		out vec4 outcolor;
		out vec4 outgeneral0;

		// const vec3 cols[] = vec3[](
		// 	vec3(1,0,0),
		// 	vec3(0,1,0),
		// 	vec3(0,0,1),
		// 	vec3(1,1,0),
		// 	vec3(1,0,1),
		// 	vec3(0,1,1),
		// 	vec3(0,0,0)
		// );

		void main() {
			// outcolor = materialColor;
			outcolor.a = materialColor.a;
			outcolor.rgb = pow(materialColor.rgb, vec3(0.5f));
			// outcolor.rgb = cols[faceID%cols.length()];

			// vec3 col;
			// float val = faceID;
			// float div = 50.f;
			// val /= div;
			// col.r = mod(val, 1.f);
			// val /= div;
			// col.g = mod(val, 1.f);
			// val /= div;
			// col.b = mod(val, 1.f);
			// outcolor.rgb = col*.5f +.5f;

			float luminance = dot(outcolor.rgb, vec3(0.2125, 0.7154, 0.0721));
			// outgeneral0 = vec4(outcolor.rgb, (1-outcolor.a) * pow(luminance, 1.0f));
			outgeneral0 = vec4(outcolor.rgb, 1) * (1-outcolor.a) * pow(luminance, 1.0f);
			// outgeneral0 = vec4(pow(luminance, 2.f));
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

		void main() {
			if(vlifetime <= 0.f) discard;

			// vec2 diff = vec2(.5f) - gl_PointCoord;
			// float dist = .6f-(abs(diff.x)+abs(diff.y))/*.5f - length()*/;
			// dist = pow(clamp(dist, 0.f, 1.f), 0.9f);
			float dist = 0.35f;
			float a = sin(radians(vlifetime*180.f)) * dist;
			outcolor = vec4(materialColor*a, a);
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
extern bool transformFeedbackAvailable;

u32 windowWidth;
u32 windowHeight;

Entity* playerEntity;

namespace {
	u32 cursorTextures[2];
	u32 cursorVBO;
	u32 cursorUVBO;
	
	ParticleSystem particleSystem;
	f32 particleEmitAccum = 0.f;
}

void GameInit();
void GameDeinit();
void GameUpdate(Scene*, Camera*, Framebuffer*, f32);

s32 main(s32 ac, char** av) {
	if(!InitLog()) {
		fprintf(stderr, "Error! Unable to init logging!\n");
		return 2;
	}

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		LogError("SDL Init failed\n");
		return 1;
	}

	{	SDL_version compiledVersion;
		SDL_version linkedVersion;
		SDL_VERSION(&compiledVersion);
		SDL_GetVersion(&linkedVersion);

		assert(compiledVersion.major == linkedVersion.major);
		assert(compiledVersion.minor == linkedVersion.minor);
		assert(compiledVersion.patch == linkedVersion.patch);
	}

	ParseCLOptions(ac, av);
	LoadOptions();

	windowWidth = GetIntOption("window.width");
	windowHeight = GetIntOption("window.height");
	bool fullscreen = GetBoolOption("window.fullscreen");

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	auto window = SDL_CreateWindow("Voi", 
		SDL_WINDOWPOS_CENTERED_DISPLAY(1), SDL_WINDOWPOS_UNDEFINED, 
		windowWidth, windowHeight, SDL_WINDOW_OPENGL);

	if(!window) {
		LogError("Window creation failed\n");
		return 1;
	}

	if(!InitGL(window)) return 1;

	Input::Init();
	Input::doCapture = true;

	SDL_WarpMouseInWindow(window, windowWidth/2, windowHeight/2);
	SDL_ShowCursor(0);

	if(!InitDebug()) {
		LogError("Warning! Debug draw init failed\n");
	}

	if(!InitScripting()) {
		LogError("Error! Scripting init failed!\n");
		return 1;
	}

	if(!InitEntityAnimator()) {
		LogError("Error! Entity Animator init failed!\n");
		return 1;
	}

	Camera camera;
	camera.fov = glm::radians<f32>(GetFloatOption("graphics.fov"));
	camera.aspect = (f32) windowWidth / windowHeight;
	camera.nearDist = 0.1f;
	camera.farDist = 1000.f;

	camera.projection = glm::perspective(camera.fov, camera.aspect, camera.nearDist, camera.farDist);

	playerEntity = AllocateEntity();
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

	Scene scene;
	if(!InitPhysics(&scene.physicsContext)) {
		LogError("Error! Physics init failed!\n");
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0, 0, 0, 0);
	glEnableVertexAttribArray(0);

	if(!InitEffects()) {
		LogError("Error! Effects init failed!\n");
		return 1;
	}

	CreateNamedShaderProgram(ShaderIDDefault,	defaultShaderSrc[0], defaultShaderSrc[1]);
	CreateNamedShaderProgram(ShaderIDParticles,	particleShaderSrc[0], particleShaderSrc[1]);
	CreateNamedShaderProgram(ShaderIDUI,		uiShaderSrc[0], uiShaderSrc[1]);

	f32 dt = 1.f/60.f;
	f32 fpsTimeAccum = 0.f;
	u32 fpsFrameCount = 0;

	auto beginTime = std::chrono::high_resolution_clock::now();

	bool queryingPrimitives = false;
	u32 primitiveCount = 0;
	u32 primCountQuery;
	if(transformFeedbackAvailable) {
		glGenQueries(1, &primCountQuery);
		glBeginQuery(GL_PRIMITIVES_GENERATED, primCountQuery);
		queryingPrimitives = true;
	}

	f32 multisampleLevel = 1.;
	bool filter = true;

	if(!strcmp(GetStringOption("graphics.multisample"), "indiepls")) { // NOTE: Sshhhh...
		multisampleLevel = 0.08;
		filter = false;
	}else if(GetBoolOption("graphics.multisample")){
		multisampleLevel = 2.;
	}

	FramebufferSettings mainFramebufferSettings {
		.width = u32(windowWidth*multisampleLevel),
		.height = u32(windowHeight*multisampleLevel),
		.numColorBuffers = 3,
		.hasStencil = true,
		.hasDepth = true,
		.filter = filter
	};

	// TODO: Better antialiasing method
	Framebuffer fb = CreateFramebuffer(mainFramebufferSettings);
	if(!fb.valid) {
		LogError("Error! Framebuffer creation failed!\n");
		return 1;
	}

	auto SetFullscreen = [window, &fb, multisampleLevel, filter, &camera] (bool fullscreen) {
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

		if(fb.fbo) DestroyFramebuffer(&fb);

		FramebufferSettings mainFramebufferSettings {
			.width = u32(windowWidth*multisampleLevel),
			.height = u32(windowHeight*multisampleLevel),
			.numColorBuffers = 3,
			.hasStencil = true,
			.hasDepth = true,
			.filter = filter
		};

		fb = CreateFramebuffer(mainFramebufferSettings);
		if(!fb.valid) {
			LogError("Error! Framebuffer recreation failed!\n");
			return false;
		}

		ReinitEffects();

		camera.aspect = (f32) windowWidth / windowHeight;
		camera.projection = glm::perspective(camera.fov, camera.aspect, camera.nearDist, camera.farDist);

		if(Input::doCapture && !fullscreen) {
			Input::UpdateMouse(window);
		}

		return true;
	};

	if(!SetFullscreen(fullscreen)) return 1;
	
	auto filename = GetStringOption("level.override");
	if(!filename[0]) filename = "export.voi";

	{	auto sceneData = LoadSceneData(filename);
		if(sceneData.numMeshes == 0 || sceneData.numEntities == 0) {
			LogError("Error! Empty scene!\n");
			return 1;
		}

		if(!InitScene(&scene, &sceneData)) {
			LogError("Error! Scene init failed!\n");
			return 1;
		}

		playerEntity->scene = &scene;
		if(!InitEntityPhysics(playerEntity, nullptr)) {
			LogError("Error! Entity physics init failed for player!\n");
			return 1;
		}

		InitEntity(playerEntity);

		FreeSceneData(&sceneData);
	}

	GameInit();

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

		GameUpdate(&scene, &camera, &fb, dt);

		if(transformFeedbackAvailable) {
			if(queryingPrimitives) {
				glEndQuery(GL_PRIMITIVES_GENERATED);
				queryingPrimitives = false;
			}else{
				u32 available = 0;
				glGetQueryObjectuiv(primCountQuery, GL_QUERY_RESULT_AVAILABLE, &available);

				if(available) {
					glGetQueryObjectuiv(primCountQuery, GL_QUERY_RESULT, &primitiveCount);
					glBeginQuery(GL_PRIMITIVES_GENERATED, primCountQuery);
					queryingPrimitives = true;
				}
			}
		}

		auto endFrameTime = std::chrono::high_resolution_clock::now();

		SDL_GL_SwapWindow(window);

		auto endTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::duration<f32>>(endTime-beginTime).count();

		f32 renderdt = std::chrono::duration_cast<std::chrono::duration<f32>>(endFrameTime-beginFrameTime).count();
		beginTime = endTime;
		fpsTimeAccum += dt;
		fpsFrameCount++;

		if(fpsTimeAccum > 0.1f) {
			f32 fps = fpsFrameCount/fpsTimeAccum;
			fpsFrameCount = 0;
			fpsTimeAccum = 0;

			char titleBuffer[256];
			std::snprintf(titleBuffer, 256, "Voi   |   %.ffps   |   %.2fms   |   %u primitives", fps, renderdt*1000.f, primitiveCount);
			// LogError("%.ffps   |   %.2fms\n", fps, dt*1000.f);
			SDL_SetWindowTitle(window, titleBuffer);
		}

		Input::ClearFrameState();
	}

	GameDeinit();

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

void GameInit() {
	if(!InitParticleSystem(&particleSystem, 6000)) {
		LogError("Warning! Particle system init failed\n");
	}

	// Simulate 3s of dust
	for(u32 i = 0; i < 60*3; i++){
		EmitParticles(&particleSystem, 1, glm::linearRand(4.f, 20.f), playerEntity->position);
		UpdateParticleSystem(&particleSystem, 1.f/60.f);
	}

	cursorTextures[0] = LoadTexture("data/cursor.png");
	cursorTextures[1] = LoadTexture("data/cursor2.png");

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
}

void GameDeinit() {
	DeinitParticleSystem(&particleSystem);

	glDeleteTextures(2, cursorTextures);
}

void GameUpdate(Scene* scene, Camera* camera, Framebuffer* fb, f32 dt) {
	auto forwardShader	= GetNamedShaderProgram(ShaderIDDefault);
	auto particleShader	= GetNamedShaderProgram(ShaderIDParticles);
	auto uiShader		= GetNamedShaderProgram(ShaderIDUI);

	vec3 vel = GetEntityVelocity(playerEntity);
	particleEmitAccum += (glm::length(vel)*10.f + 30.f) * dt;

	u32 numParticlesEmit = (u32) particleEmitAccum;
	particleEmitAccum -= numParticlesEmit;
	EmitParticles(&particleSystem, numParticlesEmit, glm::linearRand(4.f, 20.f), playerEntity->position + vel*2.f);
	UpdateParticleSystem(&particleSystem, dt);

	for(auto e: GetEntityIterator()) {
		UpdateEntity(e, dt);
	}

	UpdateEntityAnimator(dt);

	// Update camera position BEFORE updating physics!
	//	Player determines what layer to render based on position. 
	//	The physics update can change player position AFTER this has been calculated.
	// NOTE: A post-physics update step would also fix this
	camera->position = playerEntity->position + playerEntity->player.eyeOffset;
	camera->rotation = playerEntity->rotation * glm::angleAxis(playerEntity->player.mouseRot.y, vec3{1,0,0});
	camera->view = glm::mat4_cast(glm::inverse(camera->rotation)) * glm::translate<f32>(-camera->position);

	auto viewProjection = camera->projection * camera->view;

	// NOTE: UpdatePhysics fucks with player rotation for some reason
	UpdatePhysics(scene, dt);

	// Draw scene into framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
		EnableTargets({0, 1, 2});
		glViewport(0,0, fb->width, fb->height);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_BLEND);

		glUseProgram(forwardShader->program);
		EnableTargets({0, 2});

		glUniformMatrix4fv(forwardShader->viewProjectionLoc, 1, false, glm::value_ptr(camera->projection));
		glUniformMatrix4fv(forwardShader->modelLoc, 1, false, glm::value_ptr(mat4{}));
		glUniform4fv(forwardShader->materialColorLoc, 1, glm::value_ptr(vec4{0,0,0,1 /*fog*/}));
		DrawQuadAtFarPlane(camera->projection); // Mixes sky with fog

		RenderScene(scene, *camera, playerEntity->layers);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(particleShader->program);
		glUniform3fv(particleShader->materialColorLoc, 1, glm::value_ptr(vec3{.5}));
		glUniformMatrix4fv(particleShader->viewProjectionLoc, 1, false, 
			glm::value_ptr(viewProjection));

		// HACK: Because we're using point primitives for particles and they use pixels,
		// 	we have to adjust particle size between resolutions
		glUniform1f(glGetUniformLocation(particleShader->program, "pixelSize"), 
			fb->height*50.f/600.f);

		EnableTargets({1});
		glEnable(GL_PROGRAM_POINT_SIZE);
		glDepthMask(false);
		RenderParticleSystem(&particleSystem);
		glDepthMask(true);
		glDisable(GL_PROGRAM_POINT_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0,0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	ApplyEffectsAndDraw(fb, camera, dt);

	// Draw cursor
	glUseProgram(uiShader->program);
	glEnableVertexAttribArray(1);

	glActiveTextureVoi(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cursorTextures[playerEntity->player.lookingAtInteractive?1:0]);

	f32 a = camera->aspect;
	mat4 uiProj {
		1,0,0,0,
		0,a,0,0,
		0,0,1,0,
		0,0,0,1,
	};

	glUniform1i(uiShader->colorTexLoc, 0);
	glUniformMatrix4fv(uiShader->viewProjectionLoc, 1,
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
}