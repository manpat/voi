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

typedef Ogre::Vector2 vec2;
typedef Ogre::Vector3 vec3;
typedef Ogre::Vector4 vec4;

#endif