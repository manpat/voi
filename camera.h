#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include "common.h"

namespace Ogre {
	class Camera;
	class SceneNode;
	class Viewport;
}

class Camera {
public:
	Ogre::Camera* ogreCamera = nullptr;
	Ogre::SceneNode* cameraNode = nullptr;
	Ogre::Viewport* viewport = nullptr;

public:
	Camera(std::string name = "MainCamera");
	~Camera();

};

#endif