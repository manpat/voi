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
	
	// Doesn't work
	void SetColour(f32 r, f32 g, f32 b, f32 a) override;
	// Sets the image of the UI Object to the specified filename
	void SetImage(const std::string& filename, bool resizeObjectToImage = true);
	// Sets the size of the UI Object to the resolution of the image
	void ResizeObjectToImage();
	// Sets the image's position in screenspace, -1.0 to 1.0
	void SetPosition(f32 x, f32 y) override;
	// Sets the image's size in pixels, consider ResizeObjectToImage() for automatic sizing
	void SetSize(u32 w, u32 h) override;
	
	void SetPriority(u32 priority) override;
	// Sets whether the image is visible on screen
	void SetVisible(bool visible = true) override;
	// Sets whether the image is fixed to its pixel size, or scales with the screen resolution
	void FixedSize(bool fixed = true);
	// Sets whether the image always keeps it's correct ratio, regardless of screen ratio
	void MaintainRatio(bool maintainRatio = true);

	Ogre::AxisAlignedBox GetAABB() const override;

private:
	Ogre::Rectangle2D* rect = nullptr;
	Ogre::SharedPtr<Ogre::Material> mat;
	Ogre::Pass* matPass = nullptr;
	
	// Is the image always the specified pixel size
	bool fixedSize = true;

	bool maintainRatio = true;
	// Recalculates the images dimensions
	void CalculateTransform();
};

#endif//UIIMAGE_H