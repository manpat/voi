#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>
#include <OGRE/OgreColourValue.h>

#include "common.h"
#include "singleton.h"

class Input;
struct Camera;
struct UiImage;
struct UiManager;
struct Checkpoint;
struct HubManager;
struct BellManager;
struct AudioManager;
struct PortalManager;
struct MirrorManager;
struct EntityManager;
struct PhysicsManager;
struct HalfLifePointManager;
struct LayerRenderingManager;

// TODO: Move elsewhere
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
	std::shared_ptr<UiManager> uiManager;
	std::shared_ptr<HubManager> hubManager;
	std::shared_ptr<BellManager> bellManager;
	std::shared_ptr<AudioManager> audioManager;
	std::shared_ptr<MirrorManager> mirrorManager;
	std::shared_ptr<PortalManager> portalManager;
	std::shared_ptr<EntityManager> entityManager;
	std::shared_ptr<PhysicsManager> physicsManager;
	std::shared_ptr<HalfLifePointManager> halflifePointManager;
	std::shared_ptr<LayerRenderingManager> layerRenderingManager;

	Checkpoint* currentCheckpoint = nullptr;
	Player* player = nullptr;
	Camera* camera = nullptr;
	UiImage* cursor = nullptr;

	vec3 playerSpawnPosition;
	quat playerSpawnOrientation;
	Ogre::ColourValue skyColor;
	Ogre::ColourValue fogColor = Ogre::ColourValue::Black;
	f32 fogDensity = 0.2f;

	Ogre::ColourValue targetFogColor;
	f32 targetFogDensity;

	std::vector<SDLEventHook> sdlEventHooks;
	std::vector<SceneFileInfo> scenes;

	bool inFocus;
	bool shouldQuit;
	u32 width, height;
	u8 multisampleLevel;
	u8 fullscreen;
	u8 fovDegrees;

private:
	GameState gameState = GameState::NONE;
	std::string customLevelName;

public:
	App(const std::string& levelArg = "");
	~App();

	void Run();
	void Terminate();
	void Load(const std::string&);

	// Hooks
	void RegisterSDLHook(SDLEventHook);
	void RemoveSDLHook(SDLEventHook);

	// Setters
	void SetGameState(GameState);

	void SetFogColor(const Ogre::ColourValue&);
	void SetSkyColor(const Ogre::ColourValue&);
	void SetFogDensity(f32);

	// Getters
	s32 GetWindowWidth() const;
	s32 GetWindowHeight() const;
	GameState GetGameState() const;

	bool IsInFocus() const;

protected:
	void InitOgre();
	void LoadConfig();

	void Init();
	void Update();
	void ResetScene();
};

#endif