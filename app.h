#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "common.h"
#include "singleton.h"

class Input;
class Camera;
class AudioManager;
class PortalManager;
class EntityManager;
class PhysicsManager;

enum {
	WIDTH = 800,
	HEIGHT = 600
};

namespace Ogre {
	class Root;
	class SceneNode;
	class SceneManager;
	class RenderWindow;
};

struct App : Singleton<App> {
	typedef void (*SDLEventHook)(const SDL_Event&);
	typedef void (*Hook)();

	enum class GameState : u8 {
		MAIN_MENU,
		PLAYING,
		PAUSED
	};

	SDL_Window* sdlWindow;
	void* sdlGLContext;

	std::unique_ptr<Ogre::Root> ogreRoot;
	Ogre::SceneManager* sceneManager;
	Ogre::RenderWindow* window;
	Ogre::SceneNode* rootNode;

	std::shared_ptr<Camera> camera;
	std::shared_ptr<Input> input;
	std::shared_ptr<AudioManager> audioManager;
	std::shared_ptr<PortalManager> portalManager;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<PhysicsManager> physicsManager;

	std::vector<SDLEventHook> sdlEventHooks;
	std::vector<Hook> frameBeginHooks;
	std::vector<Hook> frameEndHooks;
	bool inFocus;

	bool shouldQuit;

private:
	GameState gameState;

public:
	App();
	~App();

	void Run();

	// Hooks
	void RegisterSDLHook(SDLEventHook);
	void RemoveSDLHook(SDLEventHook);

	void RegisterFrameBeginHook(Hook);
	void RemoveFrameBeginHook(Hook);

	void RegisterFrameEndHook(Hook);
	void RemoveFrameEndHook(Hook);

	// Setters
	void SetGameState(GameState);

	// Getters
	s32 GetWindowWidth() const;
	s32 GetWindowHeight() const;
	GameState GetGameState() const;

	bool IsInFocus() const;

protected:
	void InitOgre();

	void Init();
	void Update();
	void Terminate();
};

#endif