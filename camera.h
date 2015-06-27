#ifndef CAMERA_H
#define CAMERA_H

#include <OGRE/OgreCamera.h>
#include <string>

class Camera {
public:
	Ogre::Camera* ogreCamera;
	Ogre::SceneNode* cameraNode;
	Ogre::Viewport* viewport;

public:
	Camera(std::string name = "main");
	~Camera();

};

#endif