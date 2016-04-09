#include "common.h"
#include "input.h"

std::map<s32,s32> Input::keyStates;
std::map<s32,s32> Input::mouseStates;
std::map<s32, s32> Input::controllerStates;
vec2 Input::mouseDelta = vec2{0,0};

Input::MappedCode Input::mappings[MappingName::Count] = {
	// Keyboard, Mouse, Controller
	{SDLK_RETURN, -1, Input::JoyButtonA},			// Select
	{SDLK_ESCAPE, -1, Input::JoyButtonB},			// Cancel
	{SDLK_w, -1, -1},								// Forward
	{SDLK_s, -1, -1},								// Backward
	{SDLK_a, -1, -1},								// Left
	{SDLK_d, -1, -1},								// Right
	{SDLK_LSHIFT, -1, Input::JoyButtonRB},			// Boost
	{SDLK_SPACE, -1, Input::JoyButtonA},			// Jump
	{SDLK_e, SDL_BUTTON_LEFT, Input::JoyButtonX}	// Interact
};
SDL_Joystick* Input::controller;
s32 Input::controllerIndex = -1;
f32 Input::LXAxis = 0.0f;
f32 Input::LYAxis = 0.0f;
f32 Input::RXAxis = 0.0f;
f32 Input::RYAxis = 0.0f;

bool Input::doCapture = true;

void Input::Init(){
	SDL_Init(SDL_INIT_JOYSTICK);

	printf("Joysticks found: %d\n", SDL_NumJoysticks());

	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if ((controller = SDL_JoystickOpen(i))) {
			controllerIndex = i;
			break;
		}
	}

	if(controller){
		printf("Joystick %d connect\n", controllerIndex);
	}else{
		puts("Joystick unable to connect");
	}
}

void Input::Deinit(){
	if (controller) {
		SDL_JoystickClose(controller);
	}
}

void Input::InjectSDLEvent(const SDL_Event& e){
	switch(e.type){
	case SDL_KEYUP:
		// Save state and inform of change
		keyStates[e.key.keysym.sym] = Input::Up | Input::ChangedThisFrameFlag;
		break;

	case SDL_KEYDOWN:
		// Ignore repeats
		if(e.key.repeat == 0)
			// Save state and inform of change
			keyStates[e.key.keysym.sym] = Input::Down | Input::ChangedThisFrameFlag;
		break;

	case SDL_MOUSEBUTTONUP:
		// Save state and inform of change
		mouseStates[e.button.button] = Input::Up | Input::ChangedThisFrameFlag;
		break;

	case SDL_MOUSEBUTTONDOWN:
		// Save state and inform of change
		mouseStates[e.button.button] = Input::Down | Input::ChangedThisFrameFlag;
		break;

	case SDL_JOYAXISMOTION:
		switch (e.jaxis.axis) {
		case JoyAxisLX:
			LXAxis = (2 * ((f32)e.jaxis.value + 32768)) / 65535 - 1;
			break;

		case JoyAxisLY:
			LYAxis = (2 * ((f32)e.jaxis.value + 32768)) / 65535 - 1;
			break;

		case JoyAxisRX:
			RXAxis = (2 * ((f32)e.jaxis.value + 32768)) / 65535 - 1;
			break;

		case JoyAxisRY:
			RYAxis = (2 * ((f32)e.jaxis.value + 32768)) / 65535 - 1;
			break;

		}
		break;

	case SDL_JOYBUTTONUP:
		controllerStates[e.jbutton.button] = Input::Up | Input::ChangedThisFrameFlag;
		break;

	case SDL_JOYBUTTONDOWN:
		controllerStates[e.jbutton.button] = Input::Down | Input::ChangedThisFrameFlag;
		break;
	}
}

void Input::UpdateMouse(SDL_Window* window){
	// Get mouse delta from center, convert to range (-1, 1),
	//	move mouse back to center

	// The reason that this isn't being handled with SDLs event queue
	//	is the mouse warping

	if(doCapture){
		s32 mx, my;
		s32 ww, wh;

		SDL_GetMouseState(&mx, &my);
		SDL_GetWindowSize(window, &ww, &wh);

		ww &= ~1;
		wh &= ~1;
		
		mouseDelta.x = mx / static_cast<f32>(ww) * 2.f - 1.f;
		mouseDelta.y =-my / static_cast<f32>(wh) * 2.f + 1.f;
		
		SDL_WarpMouseInWindow(window, ww/2, wh/2);
	}
}

void Input::ClearFrameState(){
	// Clear all ChangedThisFrameFlag's from keyStates
	for(auto& kv: keyStates){
		kv.second &= ~Input::ChangedThisFrameFlag;
	}
	for(auto& kv: mouseStates){
		kv.second &= ~Input::ChangedThisFrameFlag;
	}
}

/*

	  ,ad8888ba,
	 d8"'    `"8b              ,d      ,d
	d8'                        88      88
	88             ,adPPYba, MM88MMM MM88MMM ,adPPYba, 8b,dPPYba, ,adPPYba,
	88      88888 a8P_____88   88      88   a8P_____88 88P'   "Y8 I8[    ""
	Y8,        88 8PP"""""""   88      88   8PP""""""" 88          `"Y8ba,
	 Y8a.    .a88 "8b,   ,aa   88,     88,  "8b,   ,aa 88         aa    ]8I
	  `"Y88888P"   `"Ybbd8"'   "Y888   "Y888 `"Ybbd8"' 88         `"YbbdP"'


*/
vec2 Input::GetMouseDelta(){
	return mouseDelta;
}

bool Input::GetButton(s32 k) {
	// Get only the raw "up or down" state
	return mouseStates[k] & 1;
}
bool Input::GetButtonDown(s32 k) {
	// Get state and return if it is down and has changed this frame
	return mouseStates[k] == (Input::ChangedThisFrameFlag|Input::Down);
}
bool Input::GetButtonUp(s32 k) {
	// Get state and return if it is up and has changed this frame
	return mouseStates[k] == (Input::ChangedThisFrameFlag|Input::Up);
}


bool Input::GetKey(s32 k){
	// Get only the raw "up or down" state
	return keyStates[k] & 1;
}

bool Input::GetKeyDown(s32 k){
	// Get state and return if it is down and has changed this frame
	return keyStates[k] == (Input::ChangedThisFrameFlag|Input::Down);
}

bool Input::GetKeyUp(s32 k){
	// Get state and return if it is up and has changed this frame
	return keyStates[k] == (Input::ChangedThisFrameFlag|Input::Up);
}

bool Input::GetControllerButton(s32 k) {
	return controllerStates[k] & 1;
}

bool Input::GetControllerButtonDown(s32 k) {
	return controllerStates[k] == (Input::ChangedThisFrameFlag | Input::Down);
}

bool Input::GetControllerButtonUp(s32 k) {
	return controllerStates[k] == (Input::ChangedThisFrameFlag | Input::Up);
}

bool Input::GetMapped(s32 k) {
	assert(k < Input::Count && k >= 0);

	auto& map = mappings[k];

	if (map.keyCode > 0 && GetKey(map.keyCode)) {
		return true;
	}
	if (map.mouseCode > 0 && GetButton(map.mouseCode)) {
		return true;
	}
	if (map.controllerCode > 0 && GetControllerButton(map.controllerCode)) {
		return true;
	}
	return false;
}

bool Input::GetMappedDown(s32 k) {
	assert(k < Input::Count && k >= 0);

	auto& map = mappings[k];

	if (map.keyCode > 0 && GetKeyDown(map.keyCode)) {
		return true;
	}
	if (map.mouseCode > 0 && GetButtonDown(map.mouseCode)) {
		return true;
	}
	if (map.controllerCode > 0 && GetControllerButtonDown(map.controllerCode)) {
		return true;
	}
	return false;
}

bool Input::GetMappedUp(s32 k) {
	assert(k < Input::Count && k >= 0);

	auto& map = mappings[k];

	if (map.keyCode > 0 && GetKeyUp(map.keyCode)) {
		return true;
	}
	if (map.mouseCode > 0 && GetButtonUp(map.mouseCode)) {
		return true;
	}
	if (map.controllerCode > 0 && GetControllerButtonUp(map.controllerCode)) {
		return true;
	}
	return false;
}