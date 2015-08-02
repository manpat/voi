#ifndef CAMERA_H
#define CAMERA_H

#include <string>

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

	float cameraYaw = 0.f;
	float cameraPitch = 0.f;

public:
	Camera(std::string name = "main");
	~Camera();

};

#endif