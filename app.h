#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include "common.h"
#include "singleton.h"

class Input;
struct Camera;
struct ControlMap;
class AudioManager;
class PortalManager;
struct EntityManager;
struct PhysicsManager;

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

struct SDL_Window;
union SDL_Event;

struct App : Singleton<App> {
	typedef void (*SDLEventHook)(const SDL_Event&);
	typedef void (*Hook)();

	enum class GameState : u8 {
		NONE,
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
	std::shared_ptr<ControlMap> controlMap;
	std::shared_ptr<AudioManager> audioManager;
	std::shared_ptr<PortalManager> portalManager;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<PhysicsManager> physicsManager;

	std::vector<SDLEventHook> sdlEventHooks;

	bool inFocus;
	bool shouldQuit;

private:
	GameState gameState = GameState::NONE;

public:
	App();
	~App();

	void Run();

	// Hooks
	void RegisterSDLHook(SDLEventHook);
	void RemoveSDLHook(SDLEventHook);

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