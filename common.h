#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <cmath>
#include <memory>
#include <cstdint>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

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

#define GLEW_STATIC
#include "glew.h"

#endif