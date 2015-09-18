#include "app.h"
#include <OGRE/OgreException.h>
#include <string>

extern void unittest_Entity();

#ifdef _WIN32
	#undef main
#endif
s32 main(int argc, char** argv){
	try{
		// App().Run();
		unittest_Entity();

	}catch(const Ogre::Exception& e){
		std::cout << "OGRE Exception!\n" << e.what() << std::endl;
		std::cin.get();
		return 1;
	}catch (const std::exception& e){
		std::cout << "std Exception!\n" << e.what() << std::endl;
		std::cin.get();
		return 2;
	}catch(const std::string& e){
		std::cout << "FUCK Exception!\n" << e << std::endl;
		std::cin.get();
		return 3;
	}catch(const char* e){
		std::cout << "FUCK Exception!\n" << e << std::endl;
		std::cin.get();
		return 4;
	}catch(...){
		std::cout << "HOLY SHIT SOME FUCKED UP SHIT HAPPENED HOLY SHIT!" << std::endl;
		std::cin.get();
		return 5;
	}

	return 0;
}