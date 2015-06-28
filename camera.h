#ifndef CAMERA_H
#define CAMERA_H

#include <OGRE/OgreCamera.h>
#include <string>

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