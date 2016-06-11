#include "voi.h"

namespace {
	#define SHADER(x) "#version 130\n" #x
	const char* postShaderSrc[] = {
		SHADER(
			in vec2 vertex;
			out vec2 uv;

			void main() {
				gl_Position = vec4(vertex, 0, 1);
				uv = vertex * 0.5f + 0.5f;
			}
		),
		SHADER(
			uniform sampler2D depthTex;
			uniform sampler2D colorTex;
			uniform sampler2D general0Tex;
			uniform mat4 viewProjection;
			uniform vec4 fogColor;
			uniform float fogDistance;
			in vec2 uv;

			out vec4 outcolor;

			void main() {
				vec4 color = texture2D(colorTex, uv);
				vec4 particle = texture2D(general0Tex, uv);
				float depth = texture2D(depthTex, uv).r * 2.f - 1.f;

				vec4 wp = inverse(viewProjection) * vec4(uv*2 - 1, depth*0.5 + 0.5, 1);
				depth = length(wp/wp.w); // Distance from eye

				float fogmix = 1-clamp(pow(depth / fogDistance, fogColor.a), 0, 1);

				// vec3 fogcolor = vec3(0.1);
				// vec3 fogcolor = vec3(1, .6, .2) * 0.1;
				// vec3 fogColor = vec3(0.25, 0.0, 0.1)*0.2;
				outcolor.rgb = clamp(color.rgb*fogmix + fogColor.rgb*(1-fogmix), 0, 1);
				outcolor.rgb += particle.rgb * particle.a;
				outcolor.a = 1;

				// float step = 8.f;
				// outcolor.rgb = floor(outcolor.rgb*step)/step;
			}
		)
	};

	ShaderProgram* shaderProgram;

	vec3 fogColor;
	f32 fogDistance;
	f32 fogDensity;
}

bool InitEffects() {
	shaderProgram = CreateNamedShaderProgram(ShaderIDPost, postShaderSrc[0], postShaderSrc[1]);
	fogColor = vec3{0.25, 0.0, 0.1}*0.2f;
	fogDistance = 150.f;
	fogDensity = 0.4f;

	return true;
}

void ApplyEffects(Framebuffer* fb, const Camera* camera, f32 dt) {
	static f32 t = 0.f;
	t += dt/4.f;

	fogDistance = 150.f + sin(t)*120.f;
	fogDensity = 0.5f + sin(t)*0.3f;
	fogColor = glm::mix(vec3{0.25, 0.0, 0.1}, vec3{0.f, 0.1f, 0.2f}, sin(t)*0.5f + 0.5f) * 0.2f;

	glUseProgram(shaderProgram->program);

	// Draw scene with post effects
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb->targets[FBTargetDepthStencil]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb->targets[FBTargetColor]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fb->targets[FBTargetGeneral0]);

	glUniform1i(shaderProgram->depthTexLoc, 0);
	glUniform1i(shaderProgram->colorTexLoc, 1);
	glUniform1i(shaderProgram->general0TexLoc, 2);
	glUniformMatrix4fv(shaderProgram->viewProjectionLoc, 1, false, glm::value_ptr(camera->projection));

	u32 fogColorLoc = glGetUniformLocation(shaderProgram->program, "fogColor");
	u32 fogDistanceLoc = glGetUniformLocation(shaderProgram->program, "fogDistance");
	glUniform4fv(fogColorLoc, 1, glm::value_ptr(vec4{fogColor, fogDensity}));
	glUniform1f(fogDistanceLoc, fogDistance);

	DrawFullscreenQuad();
}