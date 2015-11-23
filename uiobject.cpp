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

void UiObject::SetColour() {
	
}

void UiObject::SetHoverable(bool h) {
	hoverable = h;
}

void UiObject::SetPosition(f32 x, f32 y) {
	position.x = x;
	position.y = y;
}

void UiObject::SetSize(u32 w, u32 h) {
	size.x = w;
	size.y = h;
}

void UiObject::SetVisible() {
	
}

Ogre::AxisAlignedBox UiObject::GetAABB() {
	Ogre::AxisAlignedBox aab;
	return aab;
}

const std::string& UiObject::GetName() const {
	return node->getName();
}

vec2 UiObject::GetPosition() {
	return position;
}

vec2 UiObject::GetSize() {
	return size;
}