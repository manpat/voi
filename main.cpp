#include "OGRE/OgreRoot.h"
#include "OGRE/OgreRenderSystem.h"
#include "OGRE/OgreRenderWindow.h"
#include "OGRE/OgreRenderQueueListener.h"
#include "OGRE/OgreManualObject.h"
#include "OGRE/OgreEntity.h"
#include "OGRE/OgreSubEntity.h"
#include "OGRE/OgreSceneManager.h"
#include "OGRE/OgreWindowEventUtilities.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <memory>
#include <exception>
#include <string>
#include <cstdio>
#include <map>

// Helper for input
template<typename K, typename V>
V findin(const std::map<K,V>& m, K key, V defaultValue){
	auto it = m.find(key);
	if(it == m.end()) return defaultValue;

	return it->second;
}

int main(){
	try{
		enum {
			WIDTH = 800,
			HEIGHT = 600
		};

		/////////////////////////////// TODO: SDL CHECKS GO HERE ////////////////////////////////
		SDL_Init(SDL_INIT_VIDEO);
		auto sdlwindow = SDL_CreateWindow("TitleTitleTitle",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          WIDTH, HEIGHT,
                          SDL_WINDOW_OPENGL);

		auto sdlglcontext = SDL_GL_CreateContext(sdlwindow);

		SDL_WarpMouseInWindow(sdlwindow, WIDTH/2, HEIGHT/2);
		SDL_ShowCursor(false);
		/////////////////////////////// TODO: SDL CHECKS GO HERE ////////////////////////////////

		Ogre::String pluginFileName("plugins.cfg"), configFileName(""), logFileName("ogre.log");
		std::unique_ptr<Ogre::Root> ogreRoot(new Ogre::Root(pluginFileName, configFileName, logFileName));

		auto renderSystemList = ogreRoot->getAvailableRenderers();
		if(renderSystemList.size() == 0){
			Ogre::LogManager::getSingleton().logMessage("Sorry, no rendersystem was found.");
			return 1;
		}

		auto renderSystem = renderSystemList[0];
		ogreRoot->setRenderSystem(renderSystem);

		ogreRoot->initialise(false, "", "");

		// This is for informing Ogre of SDL2's opengl context
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
		auto window = ogreRoot->createRenderWindow("", WIDTH, HEIGHT, false /*fullscreen*/, &windowParams);

		auto scene = ogreRoot->createSceneManager(Ogre::ST_GENERIC, "ASceneManager");
		auto rootNode = scene->getRootSceneNode();

		auto camera = scene->createCamera("Steve");
		auto cameraNode = rootNode->createChildSceneNode("CameraName");
		cameraNode->attachObject(camera);

		// Camera/Viewport setup
		{
			auto size = 1.f;
			auto start = (1.f-size)*0.5f;
			auto viewport = window->addViewport(camera, 
				100 /*z order*/, 
				start /* left */, start /* top */,
				size /* width */, size /* height */);

			viewport->setAutoUpdated(true);
			auto g = 0.1;
			viewport->setBackgroundColour(Ogre::ColourValue(g,g,g));
			camera->setAspectRatio(
				static_cast<float>(viewport->getActualWidth())
				/ static_cast<float>(viewport->getActualHeight()));

			camera->setNearClipDistance(0.1f);
			camera->setFarClipDistance (1000.f);
		}

		Ogre::SceneNode* stencilNode = nullptr;
		Ogre::SceneNode* sceneNode = nullptr;
		{
			Ogre::ManualObject* thing = nullptr;
			Ogre::String objName = "Steve";
			thing = scene->createManualObject(objName);
			thing->setDynamic(false); // Static geometry

			float p = 1.0, m = -1.0;
			float mix = 0.3f;
			auto colour = Ogre::ColourValue(.8,.1,.1,1);
			auto c2 = colour*(1.f-mix) + Ogre::ColourValue::White*mix;

			thing->begin("BaseWhiteNoLighting" /* Material name*/, Ogre::RenderOperation::OT_TRIANGLE_LIST);
				thing->position(m,m,m); // Vertex 0 (The point)
				thing->colour(colour);
				thing->position(m,p,m); // Vertex 1 (Up)
				thing->colour(c2);
				thing->position(p,m,m); // Vertex 2 (Right)
				thing->colour(colour);
				thing->position(m,m,p); // Vertex 3 (Toward camera)
				thing->colour(colour);

				thing->triangle(0, 1, 2);
				thing->triangle(0, 2, 3);
				thing->triangle(0, 3, 1);
				thing->triangle(1, 3, 2);
			thing->end();

			Ogre::String meshName = "Dave";
			thing->convertToMesh(meshName);

			sceneNode = rootNode->createChildSceneNode();

			auto ent = scene->createEntity(meshName);
			auto entNode = sceneNode->createChildSceneNode();
			entNode->attachObject(ent);
			entNode->scale(0.3f, 0.3f, 0.3f);
			entNode->translate(0.f, 0.f, -4.f);
		}

		window->setActive(true);
		window->setAutoUpdated(false);

		ogreRoot->clearEventTimes();

		float cameraYaw = 0.f;
		float cameraPitch = 0.f;

		std::map<int, int> keyStates;
		// This flag is for indicating that a key changed during a frame
		//	Can be used for triggering things that should only happen once per 
		//	key press.
		enum {ChangedThisFrameFlag = 1<<8};

		float t = 0.0f;
		auto dt = 0.04f;

		while(!window->isClosed()){
			window->update(false);
			
			// I'm pretty sure messagePump does nothing since SDL is handling all 
			//	window and input events
			// Ogre::WindowEventUtilities::messagePump();

			SDL_Event e;
			while(SDL_PollEvent(&e)){
				if(e.type == SDL_QUIT
				|| e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE){

					window->destroy();
					break;
				}

				switch(e.type){
				case SDL_KEYUP:
					keyStates[e.key.keysym.sym] = 0 | ChangedThisFrameFlag;
					break;

				case SDL_KEYDOWN:
					if(e.key.repeat == 0) keyStates[e.key.keysym.sym] = 1 | ChangedThisFrameFlag;
					break;
				}
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

				SDL_WarpMouseInWindow(sdlwindow, WIDTH/2, HEIGHT/2);
			}

			t += dt;

			auto nyaw =  -mdx * dt * 13.f;
			auto npitch = mdy * dt * 13.f;	
			if(abs(cameraPitch + npitch) < M_PI/4.0f) { // Convoluted clamp
				cameraPitch += npitch;
			}
			cameraYaw += nyaw;

			// Rotate camera
			auto oriYaw = Ogre::Quaternion(Ogre::Radian(cameraYaw), Ogre::Vector3::UNIT_Y);
			auto ori = Ogre::Quaternion(Ogre::Radian(cameraPitch), oriYaw.xAxis()) * oriYaw;
			cameraNode->setOrientation(ori);

			// Move with WASD, based on look direction
			if(findin(keyStates, (int)SDLK_w, 0)){
				cameraNode->translate(-oriYaw.zAxis() * dt);
			}else if(findin(keyStates, (int)SDLK_s, 0)){
				cameraNode->translate(oriYaw.zAxis() * dt);
			}

			if(findin(keyStates, (int)SDLK_a, 0)){
				cameraNode->translate(-oriYaw.xAxis() * dt);
			}else if(findin(keyStates, (int)SDLK_d, 0)){
				cameraNode->translate(oriYaw.xAxis() * dt);
			}

			// Close window on ESC
			if(findin(keyStates, (int)SDLK_ESCAPE, 0)){
				window->destroy();
				break;
			}

			ogreRoot->renderOneFrame();
			SDL_GL_SwapWindow(sdlwindow);

			// Clear all ChangedThisFrameFlag's from keyStates
			for(auto& kv: keyStates){
				kv.second &= ~ChangedThisFrameFlag;
			}
		}

	}catch(const Ogre::Exception& e){
		return 1;
	}catch(const std::exception& e){
		return 1;
	}

	return 0;
}