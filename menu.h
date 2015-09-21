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
	Ogre::SceneNode* m_node = nullptr;
	Ogre::ManualObject* m_quad = nullptr;
	f32 m_delta = 0.f;
};

#endif // MENU_H