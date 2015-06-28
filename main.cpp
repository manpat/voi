#include "app.h"

int main(){
	try{
		App app;
		app.Run();

	}catch(const Ogre::Exception& e){
		return 1;
	}catch(const std::exception& e){
		return 1;
	}

	return 0;
}