#include "app.h"
#include <OGRE/OgreException.h>

int main(){
	try{
		App().Run();

	}catch(const Ogre::Exception& e){
		std::cerr << "OGRE Exception!\n" << e.what() << std::endl;
		return 1;
	}catch(const std::exception& e){
		std::cerr << "std Exception!\n" << e.what() << std::endl;
		return 1;
	}

	return 0;
}