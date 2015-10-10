#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <string>

struct App;

struct SceneLoaderInterface {
	virtual void Load(const std::string&, App*) = 0;
	virtual void Unload(App*) = 0;
};

#endif