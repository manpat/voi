#include "menu.h"
#include "app.h"
#include "input.h"

void Menu::Init() {
	// TODO: Load menu scene
}

void Menu::Uninit() {
	// TODO: Unload menu scene
}

void Menu::Update() {
	if (Input::GetKey(SDLK_RETURN)) {
		//App::GetSingleton()->gameState = App::GameState::PLAYING;
		App::GetSingleton()->SetGameState(App::GameState::PLAYING);
	} else if (Input::GetKeyDown(SDLK_ESCAPE)) {
		App::GetSingleton()->shouldQuit = true;
	}
}