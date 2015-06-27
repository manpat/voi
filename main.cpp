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

#include "app.h"
#include "camera.h" // Temporary

#include <memory>
#include <exception>
#include <string>
#include <cstdio>
#include <map>

enum {
	RENDER_QUEUE_PORTAL = Ogre::RENDER_QUEUE_6,
	RENDER_QUEUE_PORTALSCENE = Ogre::RENDER_QUEUE_7,
};

class StencilQueueListener : public Ogre::RenderQueueListener {
public:
	StencilQueueListener(){}
	~StencilQueueListener(){}

	void renderQueueStarted(uint8_t queueId, const Ogre::String& invocation, bool& skipThisInvocation) override { 
		if(queueId == RENDER_QUEUE_PORTAL){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->clearFrameBuffer(Ogre::FBT_STENCIL, Ogre::ColourValue::Black, 1.0, 0xFF);
			rs->setStencilCheckEnabled(true);
			rs->_setColourBufferWriteEnabled(false, false, false, false);
			rs->setStencilBufferParams(
				Ogre::CMPF_ALWAYS_PASS, // compare
				0x1, // refvalue
				0xFFFFFFFF, // compare mask
				0xFFFFFFFF, // write mask
				Ogre::SOP_REPLACE, Ogre::SOP_KEEP, // stencil fail, depth fail
				Ogre::SOP_REPLACE, // stencil pass + depth pass
				false); // two-sided operation? no

		}else if(queueId == RENDER_QUEUE_PORTALSCENE){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->clearFrameBuffer(Ogre::FBT_DEPTH, Ogre::ColourValue::Black, 1.0, 0xFF);
			rs->_setColourBufferWriteEnabled(true, true, true, true);
			rs->setStencilCheckEnabled(true);
			rs->setStencilBufferParams(Ogre::CMPF_EQUAL, 0x1, 0xFFFFFFFF, 0xFFFFFFFF,
				Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP, false);
		}
	}

	void renderQueueEnded(uint8_t queueId, const Ogre::String& invocation, bool& repeatThisInvocation) override {
		if(queueId == RENDER_QUEUE_PORTAL){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->setStencilCheckEnabled(false);
			rs->setStencilBufferParams();

		}else if(queueId == RENDER_QUEUE_PORTALSCENE){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->setStencilCheckEnabled(false);
			rs->setStencilBufferParams();
		}
	}
};

template<typename K, typename V>
V findin(const std::map<K,V>& m, K k, V dv){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}

int main(){
	try{
		/////////////////////////////// TODO: SDL CHECKS GO HERE ////////////////////////////////
		App app;
		app.Run();
		// return 0;
		/////////////////////////////// TODO: SDL CHECKS GO HERE ////////////////////////////////

		auto window = app.window;
		auto sceneManager = app.sceneManager;
		auto rootNode = app.rootNode;
		auto& ogreRoot = app.ogreRoot;

		auto camera = app.camera->ogreCamera;
		auto cameraNode = app.camera->cameraNode;

		Ogre::SceneNode* stencilNode = nullptr;
		Ogre::SceneNode* sceneNode = nullptr;
		{
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
		}

		sceneManager->addRenderQueueListener(new StencilQueueListener());

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

				SDL_WarpMouseInWindow(app.sdlWindow, WIDTH/2, HEIGHT/2);
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

			float boost = 1.f;

			if(findin(keyStates, (int)SDLK_LSHIFT, 0)){
				boost = 2.f;
			}

			// Move with WASD, based on look direction
			if(findin(keyStates, (int)SDLK_w, 0)){
				cameraNode->translate(-oriYaw.zAxis() * dt * boost);
			}else if(findin(keyStates, (int)SDLK_s, 0)){
				cameraNode->translate(oriYaw.zAxis() * dt * boost);
			}

			if(findin(keyStates, (int)SDLK_a, 0)){
				cameraNode->translate(-oriYaw.xAxis() * dt * boost);
			}else if(findin(keyStates, (int)SDLK_d, 0)){
				cameraNode->translate(oriYaw.xAxis() * dt * boost);
			}

			// Close window on ESC
			if(findin(keyStates, (int)SDLK_ESCAPE, 0)){
				window->destroy();
				break;
			}

			ogreRoot->renderOneFrame();
			SDL_GL_SwapWindow(app.sdlWindow);

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