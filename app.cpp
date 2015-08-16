#include <iostream>
#include <chrono>
#include <string>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreResourceGroupManager.h>

#include "app.h"
#include "input.h"
#include "camera.h"
#include "common.h"

App* App::instance = nullptr;

App::App(){
	instance = this;

	SDL_Init(SDL_INIT_VIDEO);
	sdlWindow = SDL_CreateWindow("TitleTitleTitle",
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

	Init();

	window->setActive(true);
	window->setAutoUpdated(false);

	ogreRoot->clearEventTimes();
	inFocus = true;
	shouldQuit = false;
}

App::~App(){
	instance = nullptr;
}

/*

	  ,ad8888ba,                      ad88888ba  88                         88
	 d8"'    `"8b              ,d    d8"     "8b ""                         88              ,d
	d8'                        88    Y8,                                    88              88
	88             ,adPPYba, MM88MMM `Y8aaaaa,   88 8b,dPPYba,   ,adPPYb,d8 88  ,adPPYba, MM88MMM ,adPPYba,  8b,dPPYba,
	88      88888 a8P_____88   88      `"""""8b, 88 88P'   `"8a a8"    `Y88 88 a8P_____88   88   a8"     "8a 88P'   `"8a
	Y8,        88 8PP"""""""   88            `8b 88 88       88 8b       88 88 8PP"""""""   88   8b       d8 88       88
	 Y8a.    .a88 "8b,   ,aa   88,   Y8a     a8P 88 88       88 "8a,   ,d88 88 "8b,   ,aa   88,  "8a,   ,a8" 88       88
	  `"Y88888P"   `"Ybbd8"'   "Y888  "Y88888P"  88 88       88  `"YbbdP"Y8 88  `"Ybbd8"'   "Y888 `"YbbdP"'  88       88
	                                                             aa,    ,88
	                                                              "Y8bbdP"
*/
App* App::GetSingleton(){
	return App::instance;
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
	Ogre::String pluginFileName("plugins.cfg"), configFileName(""), logFileName("ogre.log");
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
	SDL_GetWMInfo(&wmInfo);

	size_t winHandle = reinterpret_cast<size_t>(wmInfo.window);
	size_t winGlContext = reinterpret_cast<size_t>(wmInfo.hglrc);

	windowParams["externalWindowHandle"] = StringConverter::toString(winHandle);
	windowParams["externalGLContext"] = StringConverter::toString(winGlContext);
#else
	windowParams["currentGLContext"] = std::string("True");
#endif

	windowParams["FSAA"] = "0";
	windowParams["vsync"] = "true";
	// I'm pretty sure none of the createRenderWindow parameters actually do anything
	window = ogreRoot->createRenderWindow("", 0, 0, false /*fullscreen*/, &windowParams);

	sceneManager = ogreRoot->createSceneManager(Ogre::ST_GENERIC, "ASceneManager");
	rootNode = sceneManager->getRootSceneNode();

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Meshes", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Particles", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
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
	f32 dt = 0.f;

	while(!window->isClosed()){
		window->update(false);

		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT
			|| e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE){

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

		for(auto hook: frameBeginHooks){
			hook();
		}
		Update(dt);

		for(auto hook: frameEndHooks){
			hook();
		}

		ogreRoot->renderOneFrame();
		SDL_GL_SwapWindow(sdlWindow);

		auto end = high_resolution_clock::now();
		dt = duration_cast<duration<f32>>(end - begin).count();
		begin = end;

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

void App::RegisterFrameBeginHook(Hook h){
	if(!h) return;
	frameBeginHooks.push_back(h);
}

void App::RemoveFrameBeginHook(Hook h){
	if(!h) return;
	std::cerr << "Hook removal not implemented" << std::endl;
}

void App::RegisterFrameEndHook(Hook h){
	if(!h) return;
	frameEndHooks.push_back(h);
}

void App::RemoveFrameEndHook(Hook h){
	if(!h) return;
	std::cerr << "Hook removal not implemented" << std::endl;
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
s32 App::GetWindowWidth() const {
	return WIDTH;
}

s32 App::GetWindowHeight() const {
	return HEIGHT;
}

bool App::IsInFocus() const {
	return inFocus;
}