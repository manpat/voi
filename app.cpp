#include <iostream>
#include <chrono>
#include <string>
#include <typeinfo>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "app.h"
#include "camera.h"

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

	camera = std::make_shared<Camera>();
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
#ifdef WINDOWS
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
	window = ogreRoot->createRenderWindow("", WIDTH, HEIGHT, false /*fullscreen*/, &windowParams);

	sceneManager = ogreRoot->createSceneManager(Ogre::ST_GENERIC, "ASceneManager");
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
	float dt = 0.f;

	Init();

	while(true){
		Update(dt);

		auto end = high_resolution_clock::now();
		dt = duration_cast<duration<float>>(end - begin).count();

		std::cerr << dt << std::endl;
		break;
	}
}

/*
	                                                                                                            
	  ,ad8888ba,                                              88                                 88             
	 d8"'    `"8b                                             88                                 ""             
	d8'                                                       88                                                
	88            ,adPPYYba, 88,dPYba,,adPYba,   ,adPPYba,    88          ,adPPYba,   ,adPPYb,d8 88  ,adPPYba,  
	88      88888 ""     `Y8 88P'   "88"    "8a a8P_____88    88         a8"     "8a a8"    `Y88 88 a8"     ""  
	Y8,        88 ,adPPPPP88 88      88      88 8PP"""""""    88         8b       d8 8b       88 88 8b          
	 Y8a.    .a88 88,    ,88 88      88      88 "8b,   ,aa    88         "8a,   ,a8" "8a,   ,d88 88 "8a,   ,aa  
	  `"Y88888P"  `"8bbdP"Y8 88      88      88  `"Ybbd8"'    88888888888 `"YbbdP"'   `"YbbdP"Y8 88  `"Ybbd8"'  
	                                                                                  aa,    ,88                
	                                                                                   "Y8bbdP"                 
*/


/*
	                           
	88             88          
	88             ""   ,d     
	88                  88     
	88 8b,dPPYba,  88 MM88MMM  
	88 88P'   `"8a 88   88     
	88 88       88 88   88     
	88 88       88 88   88,    
	88 88       88 88   "Y888  
	                           
	                           
*/
void App::Init(){

}

/*
	                                                                   
	88        88                      88                               
	88        88                      88              ,d               
	88        88                      88              88               
	88        88 8b,dPPYba,   ,adPPYb,88 ,adPPYYba, MM88MMM ,adPPYba,  
	88        88 88P'    "8a a8"    `Y88 ""     `Y8   88   a8P_____88  
	88        88 88       d8 8b       88 ,adPPPPP88   88   8PP"""""""  
	Y8a.    .a8P 88b,   ,a8" "8a,   ,d88 88,    ,88   88,  "8b,   ,aa  
	 `"Y8888Y"'  88`YbbdP"'   `"8bbdP"Y8 `"8bbdP"Y8   "Y888 `"Ybbd8"'  
	             88                                                    
	             88                                                    
*/
void App::Update(float dt){

}