#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreVector4.h>

template<typename K, typename V>
V findin(const std::map<K,V>& m, K k, V dv){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}

using vec2 = Ogre::Vector2;
using vec3 = Ogre::Vector3;
using vec4 = Ogre::Vector4;

#endif