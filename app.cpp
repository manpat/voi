#include <iostream>
#include <sstream>
#include <chrono>
#include <string>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#ifdef _WIN32
#	include <SDL2/SDL_syswm.h>
#endif

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreFileSystem.h>
#include <OGRE/OgreConfigFile.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>

#include "app.h"
#include "menu.h"
#include "input.h"
#include "camera.h"
#include "common.h"
#include "apptime.h"
#include "uimanager.h"
#include "hubmanager.h"
#include "audiomanager.h"
#include "entitymanager.h"
#include "physicsmanager.h"
#include "halflifepointmanager.h"

template<> App* Singleton<App>::instance = nullptr;

App::App(const std::string& levelArg) {
	// Look for available scenes
	Ogre::FileSystemArchiveFactory fsfactory;
	auto fs = fsfactory.createInstance("GameData/Scenes", true);
	auto fsls = fs->findFileInfo("*.scene");

	for (auto& d : *fsls) {
		scenes.push_back(SceneFileInfo{ d.basename, "GameData/Scenes/" + d.path });
	}

	// Sort found scenes by name
	std::sort(scenes.begin(), scenes.end(), [](const SceneFileInfo& a, const SceneFileInfo& b) {
		return a.name < b.name;
	});

	std::cout << "Scenes found: \n";
	for (auto& s : scenes) {
		std::cout << "\t" << s.name.substr(0, s.name.find_last_of('.')) << "\n";
	}
	std::cout << std::endl;

	// For debugging
	if (levelArg.size() > 0) {
		customLevelName = levelArg;
	}
	//else {
	//	std::cout << "Load custom level? Leave blank for default (hub).\nName: ";
	//	std::getline(std::cin, customLevelName);
	//}

	// Find and parse config file
	LoadConfig();

	// Init SDL and create window
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw "SDL_Init failed";
	}

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	if (multisampleLevel > 1 && multisampleLevel < 17) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisampleLevel);
		glEnable(GL_MULTISAMPLE);
	}

	sdlWindow = SDL_CreateWindow("Anomalia",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					width, height,
					SDL_WINDOW_OPENGL);

	if(!sdlWindow) {
		throw "SDL Window creation failed";
	}

	switch (fullscreenMode) {
		case 2: // 2 = Fullscreen ("fake" fullscreen that takes the size of the desktop)
			SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
			break;
		case 1: // 1 = Fullscreen (videomode change)
			SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
			break;
		default: // 0 = Windowed
			SDL_SetWindowFullscreen(sdlWindow, 0);
			break;
	}

	sdlGLContext = SDL_GL_CreateContext(sdlWindow);
	if(!sdlGLContext) {
		throw "GL Context creation failed";
	}

	// Fullscreen setting may have changed resolution
	// 	This assumes that width or height is never > ~4million
	SDL_GetWindowSize(sdlWindow, (s32*)&width, (s32*)&height);

	// 0 is disabled, 1 is vsync
	SDL_GL_SetSwapInterval(useVsync);

	// TODO: refactor into input module
	SDL_WarpMouseInWindow(sdlWindow, width/2, height/2);
	SDL_ShowCursor(false);

	InitOgre();

	input = std::make_shared<Input>();
	uiManager = std::make_shared<UiManager>();
	hubManager = std::make_shared<HubManager>();
	audioManager = std::make_shared<AudioManager>();
	entityManager = std::make_shared<EntityManager>();
	physicsManager = std::make_shared<PhysicsManager>();
	halflifePointManager = std::make_shared<HalfLifePointManager>();

	window->setActive(true);
	window->setAutoUpdated(false);

	ogreRoot->clearEventTimes();
	inFocus = true;
	shouldQuit = false;

	//SetGameState(GameState::PLAYING);
	SetGameState(!customLevelName.empty() ? GameState::PLAYING : GameState::MAIN_MENU);
}

App::~App(){
	// This ensures that all entities are destroyed appropriately before ogre shuts down
	entityManager.reset();
}

void App::LoadConfig() {
	// TODO: More configuration
	// - Mouse sensitivity

	Ogre::FileSystemArchiveFactory fsfactory;
	auto fs = fsfactory.createInstance(".", false);

	// Specify defaults
	width = WIDTH;
	height = HEIGHT;
	multisampleLevel = 4;
	fullscreenMode = 0;
	useVsync = 1;
	fovDegrees = 60;

	hMouseSensitivity = 18000;
	vMouseSensitivity = 8000;

	// Test if config exists
	if (fs->exists("anomalia.cfg")) {
		// If so, load it and configure
		Ogre::ConfigFile conf;
		conf.load("anomalia.cfg");
		
		auto sectionStr = "Anomalia";
		auto wStr = conf.getSetting("width", sectionStr, std::to_string(width));
		auto hStr = conf.getSetting("height", sectionStr, std::to_string(height));
		auto mslStr = conf.getSetting("multisampleLevel", sectionStr, std::to_string(multisampleLevel));
		auto fsStr = conf.getSetting("fullscreen", sectionStr, std::to_string(fullscreenMode));
		auto fovStr = conf.getSetting("fov", sectionStr, std::to_string(fovDegrees));
		auto vsyncStr = conf.getSetting("vsync", sectionStr, std::to_string(useVsync));

		auto hSensStr = conf.getSetting("hMouseSensitivity", sectionStr, std::to_string(hMouseSensitivity));
		auto vSensStr = conf.getSetting("vMouseSensitivity", sectionStr, std::to_string(vMouseSensitivity));

		width = std::stol(wStr);
		height = std::stol(hStr);
		multisampleLevel = std::stoi(mslStr);
		fullscreenMode = std::stoi(fsStr);
		useVsync = std::stoi(vsyncStr);
		fovDegrees = std::stoi(fovStr);

		hMouseSensitivity = std::stol(hSensStr);
		vMouseSensitivity = std::stol(vSensStr);
	} else {
		// Otherwise, create one and write default values
		std::ostringstream sstr;

		sstr << "[Anomalia]\n";
		sstr << "width = " << WIDTH << "\n";
		sstr << "height = " << HEIGHT << "\n";
		sstr << "multisampleLevel = " << (u32)multisampleLevel << "\n\n";

		sstr << "# 0 -> windowed\n# 1 -> fullscreen\n# 2 -> fake fullscreen\n";
		sstr << "fullscreen = " << (u32)fullscreenMode << "\n";
		sstr << "vsync = " << (u32)useVsync << "\n";
		sstr << "fov = " << (u32)fovDegrees << "\n\n";

		sstr << "hMouseSensitivity = " << hMouseSensitivity << "\n";
		sstr << "vMouseSensitivity = " << vMouseSensitivity << "\n";

		auto ncfgFile = fs->create("anomalia.cfg");
		ncfgFile->write(sstr.str().data(), sstr.tellp());
	}
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
			Ogre::String pluginFileName("plugins_win_d.cfg"), logFileName("ogre_d.log");
		#else
			Ogre::String pluginFileName("plugins_win.cfg"), logFileName("ogre.log");
		#endif
	#else
		Ogre::String pluginFileName("plugins_linux.cfg"), logFileName("ogre.log");
	#endif

	auto lm = new Ogre::LogManager();
	lm->createLog(logFileName, true, false, false); // Stops ogre's dump to console
	ogreRoot = std::unique_ptr<Ogre::Root>(new Ogre::Root(pluginFileName, "", ""));

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
	windowParams["externalGLControl"] = "true";
#else
	windowParams["externalGLControl"] = "true";
	windowParams["currentGLContext"] = "true";
#endif

	windowParams["FSAA"] = "0";
	windowParams["vsync"] = useVsync > 0;
	// The only parameter that actually does anything is windowParams
	window = ogreRoot->createRenderWindow("", 0, 0, fullscreenMode > 0, &windowParams);

	sceneManager = ogreRoot->createSceneManager(Ogre::ST_INTERIOR, "SceneManager");
	rootNode = sceneManager->getRootSceneNode();

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/UI", "FileSystem", "Persistent");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("GameData/Particles", "FileSystem", "Persistent");
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
			case GameState::PAUSED: break;
			case GameState::NONE: break;
			default: break;
		}

		// Lerp fog settings
		const float smoothing = 120.f;
		fogColor = (fogColor * (smoothing-1.f) + targetFogColor) / smoothing;
		fogDensity = (fogDensity * (smoothing-1.f) + targetFogDensity) / smoothing;

		camera->viewport->setBackgroundColour(skyColor);
		sceneManager->setFog(Ogre::FOG_EXP, fogColor, fogDensity);

		// Updates systems
		Update();

		ogreRoot->renderOneFrame();
		SDL_GL_SwapWindow(sdlWindow);

		auto end = high_resolution_clock::now();
		auto dt = duration_cast<duration<f64>>(end - begin).count();
		begin = end;

		auto newTitle = "Anomalia " + std::to_string(1.0/dt)
			+ "fps\tTriangles: " + std::to_string(window->getTriangleCount());
		SDL_SetWindowTitle(sdlWindow, newTitle.data());

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

	if(gameState == GameState::MAIN_MENU) {
		// Changing away from main menu
		Menu::Inst().Terminate(this);

	} else if(gameState == GameState::PLAYING) {
		// Changing away from playing
		Terminate();
	}

	if(gs == GameState::MAIN_MENU) {
		// Changing to main menu
		Menu::Inst().Init(this);

	} else if(gs == GameState::PLAYING) {
		// Changing to playing
		Init();
	}

	// Finally set the actual game state
	gameState = gs;
}

void App::SetFogColor(const Ogre::ColourValue& ncol) {
	targetFogColor = ncol;
}

void App::SetSkyColor(const Ogre::ColourValue& ncol) {
	skyColor = ncol;
}

void App::SetFogDensity(f32 ndens) {
	targetFogDensity = ndens;
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
	return width;
}

s32 App::GetWindowHeight() const {
	return height;
}

bool App::IsInFocus() const {
	return inFocus;
}
