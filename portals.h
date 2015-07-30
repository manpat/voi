#ifndef PORTAL_H
#define PORTAL_H

#include <OGRE/OgreRenderQueueListener.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreCamera.h>
#include <string>

enum {
	RENDER_QUEUE_PORTAL = Ogre::RENDER_QUEUE_6,
	RENDER_QUEUE_PORTALSCENE = Ogre::RENDER_QUEUE_7,
};

class StencilQueueListener : public Ogre::RenderQueueListener {
public:
	Ogre::Camera* camera;
	Ogre::Plane portalClip;

public:
	StencilQueueListener(Ogre::Camera* c) : camera(c){}
	~StencilQueueListener(){}

	void renderQueueStarted(uint8_t queueId, const std::string& invocation, bool& skipThisInvocation) override { 
		if(invocation == "Portal"){
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


		}else if(invocation == "PortalScene"){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->clearFrameBuffer(Ogre::FBT_DEPTH, Ogre::ColourValue::Black, 1.0, 0xFF);
			rs->_setColourBufferWriteEnabled(true, true, true, true);
			rs->setStencilCheckEnabled(true);
			rs->setStencilBufferParams(Ogre::CMPF_EQUAL, 0x1, 0xFFFFFFFF, 0xFFFFFFFF,
				Ogre::SOP_KEEP, Ogre::SOP_KEEP, Ogre::SOP_KEEP, false);

			rs->addClipPlane(portalClip);
		}
	}

	void renderQueueEnded(uint8_t queueId, const std::string& invocation, bool& repeatThisInvocation) override {
		if(invocation == "Portal"){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->setStencilCheckEnabled(false);
			rs->setStencilBufferParams();

		}else if(invocation == "PortalScene"){
			auto rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->setStencilCheckEnabled(false);
			rs->setStencilBufferParams();
		}
	}
};


#endif