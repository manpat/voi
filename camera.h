#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include "common.h"

#include "component.h"

namespace Ogre {
	class Camera;
	class SceneNode;
	class Viewport;
}

struct Camera : Component {
	Ogre::Camera* ogreCamera = nullptr;
	Ogre::SceneNode* cameraNode = nullptr;
	Ogre::Viewport* viewport = nullptr;
	std::string name = "";

	Camera(std::string name = "MainCamera");

	void OnInit() override;
	void OnDestroy() override;
};

#endif