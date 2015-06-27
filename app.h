#ifndef APP_H
#define APP_H

#include <OGRE/OgreRoot.h>
#include <SDL2/SDL.h>
#include <memory>

class Camera;

enum {
	WIDTH = 800,
	HEIGHT = 600
};

class App {
public:
	static App* instance;

	SDL_Window* sdlWindow;
	void* sdlGLContext;

	std::unique_ptr<Ogre::Root> ogreRoot;
	Ogre::SceneManager* sceneManager;
	Ogre::RenderWindow* window;
	Ogre::SceneNode* rootNode;

	std::shared_ptr<Camera> camera;

public:
	App();
	~App();

	static App* GetSingleton();
	void Run();

protected:
	void InitOgre();

	void Init();
	void Update(float dt);
};

#endif