#ifndef CURSOR_H
#define CURSOR_H

#include "OGRE/OgreSharedPtr.h"

namespace Ogre {
	class Material;
	class Rectangle2D;
	class SceneNode;
}

class Cursor {
public: 
	Cursor();
	~Cursor();

	void Enable();
	void Disable();

private:
	Ogre::SceneNode* node = nullptr;
	Ogre::Rectangle2D* cursor = nullptr;
	Ogre::SharedPtr<Ogre::Material> mat;

	u32 diameter;
};

#endif//CURSOR_H