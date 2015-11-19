#ifndef UIOBJECT_H
#define UIOBJECT_H

#include "common.h"

struct UiObject {
public:
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
	~UiObject();

	// Position in screenspace, -1.0 to 1.0
	void SetPosition(f32 x, f32 y);
	// Size in pixels
	void SetSize(u32 w, u32 h);

	virtual void SetColour();

	void SetClickable(bool clickable = true);

	virtual void SetVisible();

	// Returns position in screenspace, -1.0 to 1.0
	virtual vec2 GetPosition();
	// Returns size in pixels
	virtual vec2 GetSize();

	void GetAABB();

	const std::string& GetName() const;

	// Ogre Properties
	Ogre::SceneNode* node;
	bool clickedLastFrame;
	bool hoveredLastFrame;

protected:
	// Properties
	vec2 position = vec2(0.0f, 0.0f);
	vec2 size = vec2(10, 10);
	Alignment alignment;

	bool clickable;
	bool hoverable;
};

#endif//UIOBJECT_H