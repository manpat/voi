#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "common.h"

class Input {
public:
	static std::map<s32, s32> keyStates;
	static std::map<s32, s32> mouseStates;
	static vec2 mouseDelta;

	enum {
		Forward = SDLK_w,
		Backward = SDLK_s,
		Left = SDLK_a,
		Right = SDLK_d,
		Boost = SDLK_LSHIFT,
		Jump = SDLK_SPACE,
		Interact = SDLK_e
	};

	enum {
		Up = 0,
		Down = 1,

		// This flag is for indicating that a key changed during a frame
		//	Can be used for triggering things that should only happen once per 
		//	key press.
		ChangedThisFrameFlag = 1<<8
	};

	enum {
		MouseLeft = SDL_BUTTON_LEFT,
		MouseMiddle = SDL_BUTTON_MIDDLE,
		MouseRight = SDL_BUTTON_RIGHT,
	};

public:
	Input();
	~Input();

	// Returns mouse delta since last frame
	static vec2 GetMouseDelta();

	// Returns if a mouse button is pressed
	static bool GetButton(s32 k);

	// Returns if a mouse button was pressed this frame
	static bool GetButtonDown(s32 k);

	// Returns if a mouse button was released this frame
	static bool GetButtonUp(s32 k);


	// Returns if a key is pressed
	static bool GetKey(s32 k);

	// Returns if a key was pressed this frame
	static bool GetKeyDown(s32 k);

	// Returns if a key was released this frame
	static bool GetKeyUp(s32 k);

	static void Update();
	static void EndFrame();
	
protected:
	static void EventHook(const SDL_Event&);
};

#endif