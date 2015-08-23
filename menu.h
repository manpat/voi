#ifndef MENU_H
#define MENU_H

namespace Ogre {
	SceneNode;
	ManualObject;
}

class App;

class Menu {
public:
	static Menu& Inst() {
		static Menu inst;	
		return inst;
	}

	void Init(App*);
	void Terminate(App*);
	void Update(App*, float/*f32*/);

private:
	Menu() {}

	Ogre::SceneNode* m_node;
	Ogre::ManualObject* m_quad;
	float m_delta;
};

#endif // MENU_H