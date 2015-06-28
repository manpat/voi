#include "helpers.h"
#include "input.h"
#include "app.h"

std::map<int,int> Input::keyStates;

Input::Input(){
	App::GetSingleton()->RegisterSDLHook(&EventHook);
	App::GetSingleton()->RegisterFrameEndHook(&FrameEndHook);
}

Input::~Input(){
	App::GetSingleton()->RemoveSDLHook(&EventHook);
	App::GetSingleton()->RemoveFrameEndHook(&FrameEndHook);
}

void Input::EventHook(const SDL_Event& e){
	switch(e.type){
	case SDL_KEYUP:
		keyStates[e.key.keysym.sym] = Input::Up | Input::ChangedThisFrameFlag;
		break;

	case SDL_KEYDOWN:
		if(e.key.repeat == 0) 
			keyStates[e.key.keysym.sym] = Input::Down | Input::ChangedThisFrameFlag;
		break;
	}
}

void Input::FrameEndHook(){
	// Clear all ChangedThisFrameFlag's from keyStates
	for(auto& kv: keyStates){
		kv.second &= ~Input::ChangedThisFrameFlag;
	}
}

bool Input::GetKey(int k){
	return findin(keyStates, k, 0) & 1;
}

bool Input::GetKeyDown(int k){
	return findin(keyStates, k, 0) == (Input::ChangedThisFrameFlag|Input::Down);
}

bool Input::GetKeyUp(int k){
	return findin(keyStates, k, 0) == (Input::ChangedThisFrameFlag|Input::Up);
}
