#include "uiobject.h"
#include <OGRE/OgreSceneManager.h>

UiObject::UiObject() {

}

UiObject::~UiObject() {

}

void UiObject::SetAlignment(Alignment a) {
	alignment = a;
}

void UiObject::SetClickable(bool c) {
	clickable = c;
}

void UiObject::SetColour(f32 r, f32 g, f32 b, f32 a) {
	
}

void UiObject::SetHoverable(bool h) {
	hoverable = h;
}

void UiObject::SetPosition(f32 x, f32 y) {
	position.x = x;
	position.y = y;
}

void UiObject::SetPriority(u32 p) {
	priority = p;
}

void UiObject::SetSize(u32 w, u32 h) {
	size.x = (f32)w;
	size.y = (f32)h;
}

void UiObject::SetVisible(bool) {
	
}

Ogre::AxisAlignedBox UiObject::GetAABB() const {
	Ogre::AxisAlignedBox aab;
	return aab;
}

const std::string& UiObject::GetName() const {
	return node->getName();
}

vec2 UiObject::GetPosition() const {
	return position;
}

u32 UiObject::GetPriority() const {
	return priority;
}

vec2 UiObject::GetSize() const {
	return size;
}