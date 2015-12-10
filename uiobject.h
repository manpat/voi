#ifndef UIOBJECT_H
#define UIOBJECT_H

#include "common.h"

struct UiObject {
	enum class Alignment : u8 {
		TopLeft,
		TopCenter,
		TopRight,
		CenterLeft,
		Center,
		CenterRight,
		BottomLeft,
		BottomCenter,
		BottomRight
	};

	UiObject();
	virtual ~UiObject();

	// Sets the location of the object's anchor. i.e. UiObject::Alignment::Center
	void SetAlignment(Alignment alignment);
	// Sets whether this object checks for mouseclicks
	void SetClickable(bool clickable = true);
	// Set's the colour of the object
	virtual void SetColour(f32 r, f32 g, f32 b, f32 a);
	// Sets whether this object is modified when being hovered over
	void SetHoverable(bool hoverable = true);
	// Position in screenspace, -1.0 to 1.0
	virtual void SetPosition(f32 x, f32 y);
	// Sets the distance away from the camera, lower number means closer, higher priority
	void SetPriority(u32 priority);
	// Size in pixels
	virtual void SetSize(u32 w, u32 h);
	// Sets whether the object is visible on screen
	virtual void SetVisible(bool);

	virtual Ogre::AxisAlignedBox GetAABB() const;

	// Returns the name this object's node was created with
	const std::string& GetName() const;
	// Returns position in screenspace, -1.0 to 1.0
	virtual vec2 GetPosition() const;
	// Returns priority, used for setting distance from camera, lower number is higher priority
	u32 GetPriority() const;
	// Returns size in pixels
	virtual vec2 GetSize() const;

	// Ogre Properties
	Ogre::SceneNode* node;
	bool clickedLastFrame;
	bool hoveredLastFrame;

protected:
	// Properties
	vec2 position = vec2(0.0f, 0.0f);
	vec2 size = vec2(128, 128);
	Alignment alignment = Alignment::Center;

	bool clickable = false;
	bool hoverable = false;

	u32 priority = 5;
};

#endif//UIOBJECT_H