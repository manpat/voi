#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include "common.h"

namespace Ogre {
	class Camera;
	class SceneNode;
	class Viewport;
}

// TODO: Make a component
struct Camera {
	Ogre::Camera* ogreCamera = nullptr;
	Ogre::SceneNode* cameraNode = nullptr;
	Ogre::Viewport* viewport = nullptr;

	Camera(std::string name = "MainCamera");
	~Camera();
};

#endif