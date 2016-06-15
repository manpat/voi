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

				// NOTE: viewProjection is actually inverse(projection)
				vec4 wp = viewProjection * vec4(uv*2 - 1, depth*0.5 + 0.5, 1);
				depth = length(wp/wp.w); // Distance from eye

				float fogmix = 1-clamp(pow(depth / fogDistance, fogColor.a), 0, 1);

				// vec3 fogcolor = vec3(0.1);
				// vec3 fogcolor = vec3(1, .6, .2) * 0.1;
				// vec3 fogColor = vec3(0.25, 0.0, 0.1)*0.2;
				outcolor.rgb = clamp(color.rgb*fogmix + fogColor.rgb*(1-fogmix), 0, 1);
				outcolor.rgb += particle.rgb * particle.a;
				outcolor.a = 1;

				// float step = 16.f;
				// outcolor.rgb = floor(outcolor.rgb*step)/step;
			}
		)
	};

	ShaderProgram* shaderProgram;

	struct EffectState {
		vec3 fogColor;
		f32 fogDistance;
		f32 fogDensity;
	};

	EffectState initialState;
	EffectState targetState;
	EffectState actualState;
	f32 effectLerp;

	Framebuffer antialiasFbo;
}

bool InitEffects() {
	shaderProgram = CreateNamedShaderProgram(ShaderIDPost, postShaderSrc[0], postShaderSrc[1]);
	initialState.fogColor = vec3{0.4, 0.0, 0.2}*0.2f;
	initialState.fogDistance = 30.f;
	initialState.fogDensity = 0.2f;

	// initialState.fogColor = vec3{0.1};
	// initialState.fogDistance = 200.f;
	// initialState.fogDensity = 0.5f;

	targetState = actualState = initialState;
	effectLerp = 0.f;

	return true;
}

void ApplyEffectsAndDraw(Framebuffer* fb, const Camera* camera, f32 dt) {
	// TODO: Adjustable lerp time would be nice
	effectLerp = glm::clamp(effectLerp+dt/8.f, 0.f, 1.f);

	actualState.fogColor = glm::mix(initialState.fogColor, targetState.fogColor, effectLerp);
	actualState.fogDensity = glm::mix(initialState.fogDensity, targetState.fogDensity, effectLerp);
	actualState.fogDistance = glm::mix(initialState.fogDistance, targetState.fogDistance, effectLerp);

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
	glUniformMatrix4fv(shaderProgram->viewProjectionLoc, 1, false, glm::value_ptr(glm::inverse(camera->projection)));

	u32 fogColorLoc = glGetUniformLocation(shaderProgram->program, "fogColor");
	u32 fogDistanceLoc = glGetUniformLocation(shaderProgram->program, "fogDistance");
	glUniform4fv(fogColorLoc, 1, glm::value_ptr(vec4{actualState.fogColor, actualState.fogDensity}));
	glUniform1f(fogDistanceLoc, actualState.fogDistance);

	DrawFullscreenQuad();

	// TODO: Make new framebuffer and use if for antialiasing
	//http://www.gamedev.net/topic/580517-nfaa---a-post-process-anti-aliasing-filter-results-implementation-details/
}

void SetTargetFogParameters(const vec3& color, f32 distance, f32 density) {
	initialState = actualState;
	targetState.fogColor = color;
	targetState.fogDistance = distance;
	targetState.fogDensity = density;
	effectLerp = 0.f;
}