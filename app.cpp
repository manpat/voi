#include <iostream>
#include <chrono>
#include <string>
#include <typeinfo>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSubEntity.h>

#include "app.h"
#include "input.h"
#include "camera.h"
#include "helpers.h"
#include "portals.h"

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

	while(!window->isClosed()){
		window->update(false);

		Update(dt);

		for(auto hook: frameEndHooks){
			hook();
		}

		ogreRoot->renderOneFrame();
		SDL_GL_SwapWindow(sdlWindow);

		auto end = high_resolution_clock::now();
		dt = duration_cast<duration<float>>(end - begin).count();
		begin = end;

		// std::cerr << dt << std::endl;
		// break;
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

void App::RegisterFrameEndHook(FrameEndHook h){
	if(!h) return;
	frameEndHooks.push_back(h);
}

void App::RemoveFrameEndHook(FrameEndHook h){
	if(!h) return;
	std::cerr << "Hook removal not implemented" << std::endl;
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
	Ogre::SceneNode* stencilNode = nullptr;
	Ogre::SceneNode* sceneNode = nullptr;

	Ogre::ManualObject* thing = nullptr;
	Ogre::ManualObject* portal = nullptr;
	Ogre::String objName = "Steve";
	thing = sceneManager->createManualObject(objName);
	thing->setDynamic(false /* Static geometry */);
	portal = sceneManager->createManualObject("Portal");
	portal->setDynamic(false /* Static geometry */);

	float p = 1.0, m = -1.0;
	float mix = 0.3f;
	auto colour = Ogre::ColourValue(.8,.1,.1,1);
	auto c2 = colour*(1.f-mix) + Ogre::ColourValue::White*mix;

	thing->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		thing->position(m,m,m);
		thing->colour(colour);
		thing->position(m,p,m);
		thing->colour(c2);
		thing->position(p,m,m);
		thing->colour(colour);
		thing->position(m,m,p);
		thing->colour(colour);

		thing->triangle(0, 1, 2);
		thing->triangle(0, 2, 3);
		thing->triangle(0, 3, 1);
		thing->triangle(1, 3, 2);
	thing->end();

	portal->begin("PortalStencil", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		portal->position(m,m,0.f);
		portal->colour(Ogre::ColourValue::White);
		portal->position(p,p,0.f);
		portal->colour(Ogre::ColourValue::White);
		portal->position(m,p,0.f);
		portal->colour(Ogre::ColourValue::White);
		portal->position(p,m,0.f);
		portal->colour(Ogre::ColourValue::White);

		portal->triangle(0,1,2);
		portal->triangle(0,3,1);
	portal->end();

	Ogre::String meshName = "Dave";
	Ogre::String resGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
	thing->convertToMesh(meshName);

	auto spread = 0.8f;
	sceneNode = rootNode->createChildSceneNode();
	for(int y = 0; y <= 10; y++)
		for(int x = 0; x <= 10; x++){
			auto ent = sceneManager->createEntity(meshName);
			ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE);
			auto entNode = sceneNode->createChildSceneNode();
			entNode->attachObject(ent);
			entNode->scale(0.3f, 0.3f, 0.3f);
			entNode->translate((x-5)*spread, (y-5)*spread, -10.f);
		}

	portal->convertToMesh("PortalMesh");

	auto portalNode = rootNode->createChildSceneNode();
	portalNode->translate(0, 0, -3.f);

	// Portal surface
	Ogre::Entity* ent = sceneManager->createEntity("PortalMesh");
	ent->setRenderQueueGroup(RENDER_QUEUE_PORTAL); // Stencil first
	ent->getSubEntity(0)->getMaterial()->setDepthCheckEnabled(true);
	ent->getSubEntity(0)->getMaterial()->setDepthWriteEnabled(false);

	stencilNode = portalNode->createChildSceneNode();
	stencilNode->attachObject(ent);

	auto portalGap = 2.0f;

	auto leftEnt = sceneManager->createEntity(meshName);
	auto entNode = portalNode->createChildSceneNode(Ogre::Vector3(-portalGap, 0.f, -1.0));
	entNode->attachObject(leftEnt);
	entNode->yaw(Ogre::Radian(M_PI));

	auto rightEnt = sceneManager->createEntity(meshName);
	entNode = portalNode->createChildSceneNode(Ogre::Vector3(portalGap, 0.f, -1.0));
	entNode->attachObject(rightEnt);
	entNode->yaw(Ogre::Radian(M_PI/2.0));

	auto backLeftEnt = sceneManager->createEntity(meshName);
	entNode = portalNode->createChildSceneNode(Ogre::Vector3(-portalGap, 0.f, 3.0));
	entNode->attachObject(backLeftEnt);
	entNode->yaw(Ogre::Radian(-M_PI/2.0));

	auto backRightEnt = sceneManager->createEntity(meshName);
	entNode = portalNode->createChildSceneNode(Ogre::Vector3(portalGap, 0.f, 3.0));
	entNode->attachObject(backRightEnt);
	// entNode->yaw(Ogre::Radian(0.f));

	sceneManager->addRenderQueueListener(new StencilQueueListener());
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
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT
		|| e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE){

			window->destroy();
			break;
		}

		for(auto evh: sdlEventHooks){
			evh(e);
		}

		// TODO: Release mouse on lose focus
	}

	float mdx = 0.f, mdy = 0.f;
	// Get mouse delta from center, convert to range (-1, 1), 
	//	move mouse back to center
	{
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		auto ww = (float) WIDTH;
		auto wh = (float) HEIGHT;

		mdx = mx / ww * 2.f - 1.f;
		mdy =-my / wh * 2.f + 1.f;

		SDL_WarpMouseInWindow(sdlWindow, WIDTH/2, HEIGHT/2);
	}

	auto nyaw =  -mdx * 2.0 * M_PI * dt * 7.f;
	auto npitch = mdy * 2.0 * M_PI * dt * 7.f;	
	if(abs(camera->cameraPitch + npitch) < M_PI/4.0f) { // Convoluted clamp
		camera->cameraPitch += npitch;
	}
	camera->cameraYaw += nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(camera->cameraYaw), Ogre::Vector3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(camera->cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->setOrientation(ori);

	float boost = 1.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost = 2.f;
	}

	// Move with WASD, based on look direction
	if(Input::GetKey(SDLK_w)){
		camera->cameraNode->translate(-oriYaw.zAxis() * dt * boost);
	}else if(Input::GetKey(SDLK_s)){
		camera->cameraNode->translate(oriYaw.zAxis() * dt * boost);
	}

	if(Input::GetKey(SDLK_a)){
		camera->cameraNode->translate(-oriYaw.xAxis() * dt * boost);
	}else if(Input::GetKey(SDLK_d)){
		camera->cameraNode->translate(oriYaw.xAxis() * dt * boost);
	}

	// Close window on ESC
	if(Input::GetKeyDown(SDLK_ESCAPE)){
		window->destroy();
		return;
	}
}