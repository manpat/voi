#ifndef BLENDER_SCENE_LOADER_H
#define BLENDER_SCENE_LOADER_H

#include "common.h"
#include "sceneloader.h"
#include <rapidxml.hpp>

struct BlenderSceneLoader : SceneLoaderInterface {
	void Load(const std::string& path, App*) override;
};

#endif