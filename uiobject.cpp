#include "uiobject.h"
#include <OGRE/OgreSceneManager.h>

UiObject::UiObject() {

}

UiObject::~UiObject() {

}

void UiObject::SetPosition(f32 x, f32 y) {

}

void UiObject::SetSize(u32 w, u32 h) {

}

void UiObject::SetColour() {

}

void UiObject::SetClickable(bool clickable) {

}

void UiObject::SetVisible() {
	
}

vec2 UiObject::GetPosition() {
	return vec2();
}

vec2 UiObject::GetSize() {
	return vec2();
}

void UiObject::GetAABB() {

}

const std::string& UiObject::GetName() const {
	return node->getName();
}