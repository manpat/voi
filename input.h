#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "common.h"

class Input {
public:
	static std::map<int, int> keyStates;
	static vec2 mouseDelta;

	// This flag is for indicating that a key changed during a frame
	//	Can be used for triggering things that should only happen once per 
	//	key press.
	enum {
		Up = 0,
		Down = 1,
		ChangedThisFrameFlag = 1<<8
	};

public:
	Input();
	~Input();

	// Returns mouse delta since last frame
	static vec2 GetMouseDelta();

	// Returns if a key is pressed
	static bool GetKey(int k);

	// Returns if a key was pressed this frame
	static bool GetKeyDown(int k);

	// Returns if a key was released this frame
	static bool GetKeyUp(int k);

protected:
	static void FrameBeginHook();
	static void EventHook(const SDL_Event&);
	static void FrameEndHook();
};

#endif