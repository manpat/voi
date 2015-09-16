#ifndef MENU_H
#define MENU_H

#include "common.h"

namespace Ogre {
	class SceneNode;
	class ManualObject;
}

class App;

class Menu {
public:
	static Menu& Inst();

	void Init(App*);
	void Terminate(App*);
	void Update(App*, f32);

private:
	Menu() : m_node{nullptr}, m_quad{nullptr}, m_delta{0.f} {};

	Ogre::SceneNode* m_node;
	Ogre::ManualObject* m_quad;
	f32 m_delta;
};

#endif // MENU_H