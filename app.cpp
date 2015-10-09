#include <iostream>
#include <chrono>
#include <string>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>

#include "app.h"
#include "menu.h"
#include "input.h"
#include "camera.h"
#include "common.h"
#include "apptime.h"

#include "audiomanager.h"
#include "entitymanager.h"
#include "physicsmanager.h"

template<> App* Singleton<App>::instance = nullptr;

App::App(){
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); 
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	sdlWindow = SDL_CreateWindow("Anomalia",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					WIDTH, HEIGHT,
					SDL_WINDOW_OPENGL);

	sdlGLContext = SDL_GL_CreateContext(sdlWindow);

	// TODO: refactor into input module
	SDL_WarpMouseInWindow(sdlWindow, WIDTH/2, HEIGHT/2);
	SDL_ShowCursor(false);

	InitOgre();

	input = std::make_shared<Input>();
	camera = std::make_shared<Camera>();
	audioManager = std::make_shared<AudioManager>();
	entityManager = std::make_shared<EntityManager>();
	physicsManager = std::make_shared<PhysicsManager>();

	window->setActive(true);
	window->setAutoUpdated(false);

	// SetGameState(GameState::PLAYING);
	SetGameState(GameState::MAIN_MENU);

	ogreRoot->clearEventTimes();
	inFocus = true;
	shouldQuit = false;
}

App::~App(){
	// This ensures that all entities are destroyed appropriately before ogre shuts down
	entityManager.reset();
}

/*

	88             88           ,ad8888ba,
	88             ""   ,d     d8"'    `"8b
	88                  88    d8'        `8b
	88 8b,dPPYba,  88 MM88MMM 88          88  ,adPPYb,d8 8b,dPPYba,  ,adPPYba,
	88 88P'   `"8a 88   88    88          88 a8"    `Y88 88P'   "Y8 a8P_____88
	88 88       88 88   88    Y8,        ,8P 8b       88 88         8PP"""""""
	88 88       88 88   88,    Y8a.    .a8P  "8a,   ,d88 88         "8b,   ,aa
	88 88       88 88   "Y888   `"Y8888Y"'    `"YbbdP"Y8 88          `"Ybbd8"'
	                                          aa,    ,88
	                                           "Y8bbdP"
*/
void App::InitOgre(){
	#ifdef _WIN32
		#ifdef _DEBUG
			Ogre::String pluginFileName("plugins_win_d.cfg"), 
				configFileName(""), logFileName("ogre_d.log");
		#else
			Ogre::String pluginFileName("plugins_win.cfg"), 
				configFileName(""), logFileName("ogre.log");
		#endif
	#else
		Ogre::String pluginFileName("plugins_linux.cfg"), 
			configFileName(""), logFileName("ogre.log");
	#endif
	ogreRoot = std::unique_ptr<Ogre::Root>(new Ogre::Root(pluginFileName, configFileName, logFileName));

	auto renderSystemList = ogreRoot->getAvailableRenderers();
	if(renderSystemList.size() == 0){
		Ogre::LogManager::getSingleton().logMessage("Sorry, no rendersystem was found.");
		throw "InitOgre failed";
	}

	auto renderSystem = renderSystemList[0];
	ogreRoot->setRenderSystem(renderSystem);
	ogreRoot->initialise(false, "", "");

	// TODO: Factor out into window module
	Ogre::NameValuePairList windowParams;
#ifdef _WIN32
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdlWindow, &wmInfo);

	size_t winHandle = reinterpret_cast<size_t>(wmInfo.info.win.window);
	size_t winGlContext = reinterpret_cast<size_t>(wglGetCurrentContext());

	windowParams["externalWindowHandle"] = std::to_string(winHandle);
	windowParams["externalGLContext"] = std::to_string(winGlContext);
	windowParams["externalGLControl"] = "True";
#else
	windowParams["currentGLContext"] = std::string("True");
#endif

	windowParams["FSAA"] = "0";
	windowParams["vsync"] = "true";
	// I'm pretty sure none of the createRenderWindow parameters actually do anything
	window = ogreRoot->createRenderWindow("", 0, 0, false /*fullscreen*/, &windowParams);

	sceneManager = ogreRoot->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
	rootNode = sceneManager->getRootSceneNode();
}

/*

	88888888ba
	88      "8b
	88      ,8P
	88aaaaaa8P' 88       88 8b,dPPYba,
	88""""88'   88       88 88P'   `"8a
	88    `8b   88       88 88       88
	88     `8b  "8a,   ,a88 88       88
	88      `8b  `"YbbdP'Y8 88       88


*/
void App::Run(){
	using namespace std::chrono;
	auto begin = high_resolution_clock::now();

	AppTime::appTime = 0.0;
	AppTime::deltaTime = 0.0;

	while(!window->isClosed()){
		window->update(false);

		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT
			||(e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)){

				window->destroy();
				break;
			}else if(e.type == SDL_WINDOWEVENT){
				if(e.window.event == SDL_WINDOWEVENT_ENTER){
					inFocus = true;
				}else if(e.window.event == SDL_WINDOWEVENT_LEAVE){
					inFocus = false;
				}
			}

			for(auto evh: sdlEventHooks){
				evh(e);
			}

			// TODO: Release mouse on lose focus
		}

		// Call the appropriate update function depending on game state
		switch (gameState) {
			case GameState::MAIN_MENU:
				Menu::Inst().Update(this, (f32)AppTime::deltaTime);
				break;
			case GameState::PLAYING: break;
			case GameState::PAUSED: throw "Paused state not implemented";
			default: throw "Invalid gamestate";
		}

		// Updates systems
		Update();

		ogreRoot->renderOneFrame();
		SDL_GL_SwapWindow(sdlWindow);

		auto newTitle = "Anomalia " + std::to_string(window->getLastFPS()) 
			+ "fps\tTriangles: " + std::to_string(window->getTriangleCount());
		SDL_SetWindowTitle(sdlWindow, newTitle.data());

		auto end = high_resolution_clock::now();
		auto dt = duration_cast<duration<f64>>(end - begin).count();
		begin = end;

		AppTime::Update(dt);

		// Just to free up the cpu a bit
		SDL_Delay(5);

		if(shouldQuit) {
			window->destroy();
			break;
		}
	}
}

/*

	88        88                         88
	88        88                         88
	88        88                         88
	88aaaaaaaa88  ,adPPYba,   ,adPPYba,  88   ,d8  ,adPPYba,
	88""""""""88 a8"     "8a a8"     "8a 88 ,a8"   I8[    ""
	88        88 8b       d8 8b       d8 8888[      `"Y8ba,
	88        88 "8a,   ,a8" "8a,   ,a8" 88`"Yba,  aa    ]8I
	88        88  `"YbbdP"'   `"YbbdP"'  88   `Y8a `"YbbdP"'


*/
void App::RegisterSDLHook(SDLEventHook h){
	if(!h) return;
	sdlEventHooks.push_back(h);
}

void App::RemoveSDLHook(SDLEventHook h){
	if(!h) return;
	std::cerr << "Hook removal not implemented" << std::endl;
}

/*

	 ad88888ba
	d8"     "8b              ,d      ,d
	Y8,                      88      88
	`Y8aaaaa,    ,adPPYba, MM88MMM MM88MMM ,adPPYba, 8b,dPPYba, ,adPPYba,
	  `"""""8b, a8P_____88   88      88   a8P_____88 88P'   "Y8 I8[    ""
	        `8b 8PP"""""""   88      88   8PP""""""" 88          `"Y8ba,
	Y8a     a8P "8b,   ,aa   88,     88,  "8b,   ,aa 88         aa    ]8I
	 "Y88888P"   `"Ybbd8"'   "Y888   "Y888 `"Ybbd8"' 88         `"YbbdP"'


*/

void App::SetGameState(GameState gs) {
	// Don't bother if already in game state
	if (gs == gameState) {
		return;
	}

	// Changing to main menu
	if (gs == GameState::MAIN_MENU) {
		Menu::Inst().Init(this);
	} else if (gameState == GameState::MAIN_MENU) {
		// Changing away from main menu
		Menu::Inst().Terminate(this);
	}

	// Changing to playing
	if (gs == GameState::PLAYING) {
		Init();
	} else if (gameState == GameState::PLAYING) {
		// Changing away from playing
		//Terminate();
	}

	// Finally set the actual game state
	gameState = gs;
}

/*

	  ,ad8888ba,
	 d8"'    `"8b              ,d      ,d
	d8'                        88      88
	88             ,adPPYba, MM88MMM MM88MMM ,adPPYba, 8b,dPPYba, ,adPPYba,
	88      88888 a8P_____88   88      88   a8P_____88 88P'   "Y8 I8[    ""
	Y8,        88 8PP"""""""   88      88   8PP""""""" 88          `"Y8ba,
	 Y8a.    .a88 "8b,   ,aa   88,     88,  "8b,   ,aa 88         aa    ]8I
	  `"Y88888P"   `"Ybbd8"'   "Y888   "Y888 `"Ybbd8"' 88         `"YbbdP"'


*/

auto App::GetGameState() const -> GameState {
	return gameState;
}

s32 App::GetWindowWidth() const {
	return WIDTH;
}

s32 App::GetWindowHeight() const {
	return HEIGHT;
}

bool App::IsInFocus() const {
	return inFocus;
}