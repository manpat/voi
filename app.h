#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include "common.h"
#include "singleton.h"

class Input;
struct Camera;
struct Checkpoint;
struct BellManager;
struct AudioManager;
struct PortalManager;
struct MirrorManager;
struct EntityManager;
struct PhysicsManager;
struct AreaTriggerManager;
struct LayerRenderingManager;

enum {
	WIDTH = 1024,
	HEIGHT = 576
};

namespace Ogre {
	class Root;
	class SceneNode;
	class SceneManager;
	class RenderWindow;
}

struct SDL_Window;
union SDL_Event;

struct Player;

struct SceneFileInfo {
	std::string name;
	std::string path;
};

struct App : Singleton<App> {
	typedef void (*SDLEventHook)(const SDL_Event&);
	typedef void (*Hook)();

	enum class GameState : u8 {
		NONE,
		MAIN_MENU,
		PLAYING,
		PAUSED
	};

	SDL_Window* sdlWindow = nullptr;
	void* sdlGLContext = nullptr;

	std::unique_ptr<Ogre::Root> ogreRoot;
	Ogre::SceneManager* sceneManager;
	Ogre::RenderWindow* window;
	Ogre::SceneNode* rootNode;

	std::shared_ptr<Input> input;
	std::shared_ptr<BellManager> bellManager;
	std::shared_ptr<AudioManager> audioManager;
	std::shared_ptr<MirrorManager> mirrorManager;
	std::shared_ptr<PortalManager> portalManager;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<PhysicsManager> physicsManager;
	std::shared_ptr<AreaTriggerManager> areaTriggerManager;
	std::shared_ptr<LayerRenderingManager> layerRenderingManager;

	Checkpoint* currentCheckpoint = nullptr;
	Player* player = nullptr;
	Camera* camera = nullptr;

	quat playerSpawnOrientation;
	vec3 playerSpawnPosition;

	std::vector<SDLEventHook> sdlEventHooks;
	std::vector<SceneFileInfo> scenes;

	bool inFocus;
	bool shouldQuit;

private:
	GameState gameState = GameState::NONE;
	std::string customLevelName;

public:
	App(const std::string& levelArg = "");
	~App();

	void Run();
	void ResetScene();
	void Load(const std::string&);

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