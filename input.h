#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <map>

#include "common.h"

class Input {
public:
	enum MappingName {
		Select,
		Cancel,
		Forward,
		Backward,
		Left,
		Right,
		Boost,
		Jump,
		Interact,
		Count
	};

	struct MappedCode {
		s32 keyCode = -1;
		s32 mouseCode = -1;
		s32 controllerCode = -1;

		MappedCode(s32 k, s32 m, s32 c) : keyCode{ k }, mouseCode{ m }, controllerCode{ c } {}
	};

	enum : u8 {
		Up = 0,
		Down = 1,

		// This flag is for indicating that a key changed during a frame
		//	Can be used for triggering things that should only happen once per
		//	key press.
		ChangedThisFrameFlag = 1<<2
	};

	enum {
		MouseLeft = SDL_BUTTON_LEFT,
		MouseMiddle = SDL_BUTTON_MIDDLE,
		MouseRight = SDL_BUTTON_RIGHT,
	};

	enum {
		JoyAxisLX = 0,
		JoyAxisLY = 1,
		JoyAxisRX = 4,
		JoyAxisRY = 5
	};

	enum {
		JoyButtonA = 1,
		JoyButtonB = 2,
		JoyButtonX = 3,
		JoyButtonRB = 6
	};

	static std::map<s32, u8> keyStates;
	static std::map<s32, u8> mouseStates;
	static std::map<s32, u8> controllerStates;
	static vec2 actualMouseDelta;
	static vec2 mouseDelta;
	static MappedCode mappings[MappingName::Count];
	static SDL_Joystick* controller;
	static f32 LXAxis, LYAxis, RXAxis, RYAxis;

	static bool doCapture;
	static bool isFocussed;

public:
	static void Init();
	static void Deinit();
	static void InjectSDLEvent(const SDL_Event&);
	static void UpdateMouse(SDL_Window*);
	static void ClearFrameState();

	// Returns mouse delta since last frame
	static vec2 GetMouseDelta();
	// TODO: Actual mouse position for menu stuff

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

	static bool GetControllerButton(s32 k);
	static bool GetControllerButtonDown(s32 k);
	static bool GetControllerButtonUp(s32 k);

	static bool GetMapped(s32 k);
	static bool GetMappedDown(s32 k);
	static bool GetMappedUp(s32 k);
};

#endif