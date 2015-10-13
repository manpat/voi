#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <cmath>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <OGRE/OgrePlane.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreVector4.h>
#include <OGRE/OgreMatrix3.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreQuaternion.h>

using vec2 = Ogre::Vector2;
using vec3 = Ogre::Vector3;
using vec4 = Ogre::Vector4;
using mat3 = Ogre::Matrix3;
using mat4 = Ogre::Matrix4;
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

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifndef PI
#define PI M_PI
#endif

template<typename K, typename V>
V findin(const std::map<K,V>& m, K k, V dv = V()){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}
template<typename K, typename V>
V findin(const std::unordered_map<K,V>& m, K k, V dv = V()){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}

template<class T, class L, class U>
T clamp(T v, L l = 0, U u = 1){
	return std::max(std::min((T)u, v), (T)l);
}

#endif