#ifndef APP_H
#define APP_H

#include <OGRE/OgreRoot.h>
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Camera;
class Input;

enum {
	WIDTH = 800,
	HEIGHT = 600
};

class App {
public:
	typedef void (*SDLEventHook)(const SDL_Event&);
	typedef void (*Hook)();

public:
	static App* instance;

	SDL_Window* sdlWindow;
	void* sdlGLContext;

	std::unique_ptr<Ogre::Root> ogreRoot;
	Ogre::SceneManager* sceneManager;
	Ogre::RenderWindow* window;
	Ogre::SceneNode* rootNode;

	std::shared_ptr<Camera> camera;
	std::shared_ptr<Input> input;

	std::vector<SDLEventHook> sdlEventHooks;
	std::vector<Hook> frameBeginHooks;
	std::vector<Hook> frameEndHooks;

public:
	App();
	~App();

	static App* GetSingleton();
	void Run();

	void RegisterSDLHook(SDLEventHook);
	void RemoveSDLHook(SDLEventHook);

	void RegisterFrameBeginHook(Hook);
	void RemoveFrameBeginHook(Hook);

	void RegisterFrameEndHook(Hook);
	void RemoveFrameEndHook(Hook);

protected:
	void InitOgre();

	void Init();
	void Update(float dt);
};

#endif