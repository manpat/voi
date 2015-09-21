#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "common.h"

class Input {
public:
	static std::map<s32, s32> keyStates;
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