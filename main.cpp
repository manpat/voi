#include "app.h"
#include <OGRE/OgreException.h>
#include <string>

#ifdef _WIN32
#undef main
#include <Windows.h>
#endif

s32 main(int na, char** aa){
	try{
		std::string level;

		if(na > 1) {
			level = std::string{aa[1]};
		}

		App app(level);

#ifdef _WIN32
		FreeConsole();
#endif

		app.Run();

	}catch(const Ogre::Exception& e){
		std::cout << "OGRE Exception!\n" << e.what() << std::endl;
		std::cin.get();
		return 1;
	}catch (const std::exception& e){
		std::cout << "std Exception!\n" << e.what() << std::endl;
		std::cin.get();
		return 2;
	}catch(const std::string& e){
		std::cout << "Exception!\n" << e << std::endl;
		std::cin.get();
		return 3;
	}catch(const char* e){
		std::cout << "Exception!\n" << e << std::endl;
		std::cin.get();
		return 4;
	}catch(...){
		std::cout << "HOLY SHIT SOME FUCKED UP SHIT HAPPENED HOLY SHIT!" << std::endl;
		std::cin.get();
		return 5;
	}

	return 0;
}