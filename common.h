#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <memory>
#include <cstdint>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreVector4.h>
#include <OGRE/OgreQuaternion.h>

template<typename K, typename V>
V findin(const std::map<K,V>& m, K k, V dv = V()){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}

using vec2 = Ogre::Vector2;
using vec3 = Ogre::Vector3;
using vec4 = Ogre::Vector4;
using quat = Ogre::Quaternion;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

#endif