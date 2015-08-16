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
	Ogre::Camera* ogreCamera;
	Ogre::SceneNode* cameraNode;
	Ogre::Viewport* viewport;

	f32 cameraYaw = 0.f;
	f32 cameraPitch = 0.f;

public:
	Camera(std::string name = "main");
	~Camera();

};

#endif