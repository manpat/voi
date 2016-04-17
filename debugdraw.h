#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include "common.h"

void InitDebugDraw();
void DrawDebug(const mat4& viewProjection);

void DebugLine(vec3,vec3,vec3 = vec3{1});
void DebugLine(vec3,vec3,vec3,vec3);

void DebugPoint(vec3,vec3 = vec3{1});

#endif