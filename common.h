#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <cstdio>
#include <cstdint>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/color_space.hpp>

using glm::ivec2;
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

template<class F>
struct constant_template {
	static constexpr F infinity = std::numeric_limits<F>::infinity();
	static constexpr F nan = std::numeric_limits<F>::quiet_NaN();
	static constexpr F pi = glm::pi<F>();
};

using constant = constant_template<f32>;
using constant64 = constant_template<f64>;

#endif