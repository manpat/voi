#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "common.h"
#include "audiomanager.h"

class Camera;
class Input;
class PortalManager;

enum {
	WIDTH = 800,
	HEIGHT = 600
};

namespace Ogre {
	class Root;
	class Entity;
	class SceneNode;
	class SceneManager;
	class RenderWindow;
	class RenderQueueInvocationSequence;
};

class App {
public:
	typedef void (*SDLEventHook)(const SDL_Event&);
	typedef void (*Hook)();

	enum class GameState : u8 {
		MAIN_MENU,
		PLAYING,
		PAUSED
	};

	void SetGameState(GameState);
	GameState GetGameState() const; // Getter

private:
	GameState gameState;

public:
	static App* instance;

	SDL_Window* sdlWindow;
	void* sdlGLContext;

	std::unique_ptr<Ogre::Root> ogreRoot;
	Ogre::SceneManager* sceneManager;
	Ogre::RenderWindow* window;
	Ogre::SceneNode* rootNode;
	AudioManager* audioManager;

	std::shared_ptr<Camera> camera;
	std::shared_ptr<Input> input;
	std::shared_ptr<PortalManager> portalManager;

	std::vector<SDLEventHook> sdlEventHooks;
	std::vector<Hook> frameBeginHooks;
	std::vector<Hook> frameEndHooks;
	bool inFocus;

	bool shouldQuit;

public:
	App();
	~App();

	static App* GetSingleton();
	void Run();

	// Hooks
	void RegisterSDLHook(SDLEventHook);
	void RemoveSDLHook(SDLEventHook);

	void RegisterFrameBeginHook(Hook);
	void RemoveFrameBeginHook(Hook);

	void RegisterFrameEndHook(Hook);
	void RemoveFrameEndHook(Hook);

	// Getters
	s32 GetWindowWidth() const;
	s32 GetWindowHeight() const;

	bool IsInFocus() const;

protected:
	void InitOgre();

	void Init();
	void Update(f32 dt);
	void Terminate();
};

#endif