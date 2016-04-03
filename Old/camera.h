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

	void SetNearClipDistance(float dist);
	void SetFarClipDistance(float dist);

	// Set the camera's background colour
	void SetBackgroundColour(Ogre::ColourValue colour);

	// Set the camera's background colour
	void SetBackgroundColour(float r, float g, float b, float a = 1.0f);
};

#endif