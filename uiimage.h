#ifndef UIIMAGE_H
#define UIIMAGE_H

#include "uiobject.h"

#include <OGRE/OgreSharedPtr.h>

namespace Ogre {
	class Rectangle2D;
	class Material;
}

struct UiImage : UiObject {
public:
	UiImage();
	~UiImage();

	void Init();

	// Sets the image of the UI Object to the specified filename
	void SetImage(const std::string& filename, bool resizeObjectToImage = true);
	// Sets the size of the UI Object to the resolution of the image
	void ResizeObjectToImage();

	void SetVisible(bool visible = true);

private:
	Ogre::Rectangle2D* rect = nullptr;
	Ogre::SharedPtr<Ogre::Material> mat;
	Ogre::Pass* matPass = nullptr;
};

#endif//UIIMAGE_H