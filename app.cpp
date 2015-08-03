#include <iostream>
#include <chrono>
#include <string>
#include <typeinfo>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreResourceGroupManager.h>

#include "app.h"
#include "input.h"
#include "camera.h"
#include "helpers.h"
#include "portalmanager.h"

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
	portalManager = std::make_shared<PortalManager>(ogreRoot.get(), camera);

	Init();

	window->setActive(true);
	window->setAutoUpdated(false);

	ogreRoot->clearEventTimes();
	inFocus = true;
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

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Meshes", "FileSystem");
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
	float dt = 0.f;

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
int App::GetWindowWidth() const {
	return WIDTH;
}

int App::GetWindowHeight() const {
	return HEIGHT;
}

bool App::IsInFocus() const {
	return inFocus;
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
	sceneManager->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(.1,.1,.1), 0.05, 10.0, 30.0);

	Ogre::ManualObject* thing = nullptr;
	Ogre::ManualObject* portal = nullptr;
	Ogre::String objName = "Steve";
	thing = sceneManager->createManualObject(objName);
	thing->setDynamic(false /* Static geometry */);

	float p = 1.0, m = -1.0;
	float mix = 0.3f;
	auto colour = Ogre::ColourValue(.8,.1,.1,1);
	auto c2 = colour*(1.f-mix) + Ogre::ColourValue::White*mix;

	thing->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		thing->position(m,m,m);
		thing->colour(colour);
		thing->position(m,p*2,m);
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

	Ogre::String meshName = "Dave";
	thing->convertToMesh(meshName);

	{
		auto spread = 0.8f;
		sceneNode2 = rootNode->createChildSceneNode();
		for(int y = 0; y <= 10; y++)
			for(int x = 0; x <= 10; x++){
				auto ent = sceneManager->createEntity("Icosphere.mesh");
				ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
				auto entNode = sceneNode2->createChildSceneNode();
				entNode->attachObject(ent);
				entNode->scale(0.3f, 0.3f, 0.3f);
				entNode->translate((x-5)*spread, (y-5)*spread, -10.f);
			}

		auto ent = sceneManager->createEntity("Icosphere.mesh");
		ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
		auto entNode = sceneNode2->createChildSceneNode();
		entNode->attachObject(ent);
		entNode->scale(2.f, 2.f, 2.f);
		entNode->translate(0, 0, 5.f);

		auto cnode = sceneNode2->createChildSceneNode();
		auto courtyard = sceneManager->createEntity("Courtyard2", "Courtyard.mesh");
		courtyard->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+1);
		cnode->attachObject(courtyard);
		cnode->scale(0.25, 0.25, 0.25);
		cnode->translate(0, -1.0, 20.0);
	}

	sceneNode1 = rootNode->createChildSceneNode();
	auto portalNode = rootNode->createChildSceneNode();
	portalNode->translate(0, 0, -3.f);

	constexpr int repetitions = 4;
	for(int z = -repetitions; z <= repetitions; z++)
		for(int x = -repetitions; x <= repetitions; x++){
			auto cnode = sceneNode1->createChildSceneNode();
			auto courtyard = sceneManager->createEntity("Courtyard.mesh");
			cnode->attachObject(courtyard);
			cnode->scale(0.25, 0.25, 0.25); // 42/4 21 10.5
			cnode->translate(x*21.0, -1.0, z*21.0-2.0);
		}

	auto door = sceneManager->createEntity("mergeDoor.mesh");
	auto doorNode = portalNode->createChildSceneNode();
	doorNode->attachObject(door);
	doorNode->scale(0.35, 0.35, 0.35);
	doorNode->translate(0, -1.0, 0);

	auto door2 = sceneManager->createEntity("mergeGate.mesh");
	auto door2Node = portalNode->createChildSceneNode();
	door2Node->attachObject(door2);
	door2Node->scale(0.4, 0.4, 0.4);
	door2Node->translate(10.0, -1.0, 0);

	auto l3ent = sceneManager->createEntity("Icosphere.mesh");
	l3ent->setRenderQueueGroup(RENDER_QUEUE_PORTALSCENE+2);

	auto l3entNode = rootNode->createChildSceneNode();
	l3entNode->attachObject(l3ent);
	l3entNode->translate(10.0, 0.0, -6.0);

	Portalify(door, 0, 1);
	Portalify(door2, 1, 2);

	sceneManager->addRenderQueueListener(portalManager.get());
	portalManager->SetLayer(0);
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
	auto md = Input::GetMouseDelta();

	auto nyaw =  -md.x * 2.0 * M_PI * dt * 7.f;
	auto npitch = md.y * 2.0 * M_PI * dt * 7.f;
	if(abs(camera->cameraPitch + npitch) < M_PI/4.0f) { // Convoluted clamp
		camera->cameraPitch += npitch;
	}
	camera->cameraYaw += nyaw;

	// Rotate camera
	auto oriYaw = Ogre::Quaternion(Ogre::Radian(camera->cameraYaw), vec3::UNIT_Y);
	auto ori = Ogre::Quaternion(Ogre::Radian(camera->cameraPitch), oriYaw.xAxis()) * oriYaw;
	camera->cameraNode->setOrientation(ori);

	float boost = 2.f;

	if(Input::GetKey(SDLK_LSHIFT)){
		boost = 4.f;
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

	if(Input::GetKeyDown('f')){
		static bool flipped = false;
		portalManager->SetLayer((int)(flipped = !flipped));
	}
}

void App::Portalify(Ogre::Entity* e, int l0, int l1){
	/////// The following process should happen during scene loading
	// subMeshes is an std::unordered_map<string, ushort>
	auto subMeshes = e->getMesh()->getSubMeshNameMap();
	auto doorpit = subMeshes.find("Portal");
	if(doorpit == subMeshes.end()) {
		std::cout << "No portal surface found" << std::endl;

		return;
	}else{
		// This assumes that subMeshes and subEntities match one to one
		auto portalEnt = e->getSubEntity(doorpit->second);
		portalEnt->getMaterial()->setSelfIllumination(Ogre::ColourValue(0.1, 0.1, 0.1)); // Skycolor
		portalEnt->getMaterial()->setCullingMode(Ogre::CULL_NONE); // Back and front face

		portalManager->AddPortal(portalEnt, l0, l1);
	}
	////////////////////////////////////////////////////////
}