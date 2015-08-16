#include "common.h"
#include "input.h"
#include "app.h"

std::map<s32,s32> Input::keyStates;
vec2 Input::mouseDelta = vec2::ZERO;

Input::Input(){
	App::GetSingleton()->RegisterSDLHook(&EventHook);
	App::GetSingleton()->RegisterFrameBeginHook(&FrameBeginHook);
	App::GetSingleton()->RegisterFrameEndHook(&FrameEndHook);
}

Input::~Input(){
	App::GetSingleton()->RemoveSDLHook(&EventHook);
	App::GetSingleton()->RemoveFrameBeginHook(&FrameBeginHook);
	App::GetSingleton()->RemoveFrameEndHook(&FrameEndHook);
}

/*
	                                                          
	88        88                         88                   
	88        88                         88                   
	88        88                         88                   
	88aaaaaaaa88  ,adPPYba,   ,adPPYba,  88   ,d8  ,adPPYba,  
	88""""""""88 a8"     "8a a8"     "8a 88 ,a8"   I8[    ""  
	88        88 8b       d8 8b       d8 8888[      `"Y8ba,   
	88        88 "8a,   ,a8" "8a,   ,a8" 88`"Yba,  aa    ]8I  
	88        88  `"YbbdP"'   `"YbbdP"'  88   `Y8a `"YbbdP"'  
	                                                          
	                                                          
*/
void Input::EventHook(const SDL_Event& e){
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
	}
}

void Input::FrameBeginHook(){
	// Get mouse delta from center, convert to range (-1, 1), 
	//	move mouse back to center

	s32 mx, my;
	SDL_GetMouseState(&mx, &my);

	auto ww = App::GetSingleton()->GetWindowWidth();
	auto wh = App::GetSingleton()->GetWindowHeight();

	mouseDelta.x = mx / static_cast<f32>(ww) * 2.f - 1.f;
	mouseDelta.y =-my / static_cast<f32>(wh) * 2.f + 1.f;

	if(App::GetSingleton()->IsInFocus())
		SDL_WarpMouseInWindow(App::GetSingleton()->sdlWindow, ww/2, wh/2);
}

void Input::FrameEndHook(){
	// Clear all ChangedThisFrameFlag's from keyStates
	for(auto& kv: keyStates){
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

bool Input::GetKey(s32 k){
	// Get only the raw "up or down" state
	return findin(keyStates, k) & 1;
}

bool Input::GetKeyDown(s32 k){
	// Get state and return if it is down and has changed this frame
	return findin(keyStates, k) == (Input::ChangedThisFrameFlag|Input::Down);
}

bool Input::GetKeyUp(s32 k){
	// Get state and return if it is up and has changed this frame
	return findin(keyStates, k) == (Input::ChangedThisFrameFlag|Input::Up);
}
