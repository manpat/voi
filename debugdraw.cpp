#include "voi.h"

#include <vector>

#define SHADER(x) "#version 130\n" #x
namespace {
	const char* debugShaderSrc[] = {
		SHADER(
			in vec3 vertex;
			in vec3 color;
			out vec3 vcolor;

			uniform mat4 viewProjection;

			void main() {
				gl_Position = viewProjection * vec4(vertex, 1);
				vcolor = color;
			}
		),
		SHADER(
			in vec3 vcolor;
			out vec4 outcolor;

			void main() {
				outcolor = vec4(vcolor, 1);
			}
		)
	};

	ShaderProgram debugProgram;
	u32 vbo;

	// Vertex, color, ...
	std::vector<vec3> lines;
	std::vector<vec3> points;
}

bool debugDrawEnabled = false;

bool InitDebugDraw() {
	vbo = 0;
	debugProgram = CreateShaderProgram(debugShaderSrc[0], debugShaderSrc[1]);

	if(!debugProgram.program) {
		puts("WARNING! Debug draw shader compilation failed!");
		return false;
	}

	glGenBuffers(1, &vbo);
	return true;
}

void DrawDebug(const mat4& viewProjection) {
	if(!debugProgram.program || !vbo) debugDrawEnabled = false;
	if(!debugDrawEnabled) return;

	glClear(GL_DEPTH_BUFFER_BIT);
	glLineWidth(4);
	glPointSize(4);
	
	glUseProgram(debugProgram.program);
	glUniformMatrix4fv(debugProgram.viewProjectionLoc, 1, false, glm::value_ptr(viewProjection));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	if(lines.size() > 0) {
		glBufferData(GL_ARRAY_BUFFER, lines.size()*sizeof(vec3), &lines[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vec3)*2, nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vec3)*2, (void*) sizeof(vec3));

		glDrawArrays(GL_LINES, 0, lines.size()/2);
	}

	if(points.size() > 0) {
		glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(vec3), &points[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vec3)*2, nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vec3)*2, (void*) sizeof(vec3));

		glDrawArrays(GL_POINTS, 0, points.size()/2);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(1);
	lines.clear();
	points.clear();
}

void DebugLine(const vec3& a, const vec3& b, const vec3& col) {
	DebugLine(a, b, col, col);
}

void DebugLine(const vec3& a, const vec3& b, const vec3& acol, const vec3& bcol) {
	if(!debugDrawEnabled) return;

	lines.push_back(a);
	lines.push_back(acol);
	lines.push_back(b);
	lines.push_back(bcol);
}

void DebugPoint(const vec3& a, const vec3& col) {
	if(!debugDrawEnabled) return;

	points.push_back(a);
	points.push_back(col);
}