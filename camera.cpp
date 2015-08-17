#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreCamera.h>

#include "camera.h"
#include "app.h"

using std::string;

Camera::Camera(string name){
	auto app = App::GetSingleton();

	ogreCamera = app->sceneManager->createCamera(name);
	cameraNode = app->rootNode->createChildSceneNode(name);
	cameraNode->attachObject(ogreCamera);
	cameraNode->translate(0, 1.0, 0);

	auto size = 1.f;
	auto start = (1.f-size)*0.5f;

	viewport = app->window->addViewport(ogreCamera,
		100 /*z order*/, 
		start /* left */, start /* top */,
		size /* width */, size /* height */);

	viewport->setAutoUpdated(true);
	auto g = 0.1;
	viewport->setBackgroundColour(Ogre::ColourValue(g,g,g)); // TODO: Expose

	ogreCamera->setAspectRatio(
		static_cast<f32>(viewport->getActualWidth())
		/ static_cast<f32>(viewport->getActualHeight()));

	ogreCamera->setNearClipDistance(0.1f); // TODO: Expose
	ogreCamera->setFarClipDistance (1000.f); // TODO: Expose
}

Camera::~Camera(){

}