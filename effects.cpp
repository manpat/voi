#include "voi.h"

namespace {
	#define SHADER(x) "#version 150\n" #x
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
			uniform mat4 prevProjView;
			uniform mat4 projection;
			uniform mat4 view;
			uniform vec4 fogColor;
			uniform float fogDistance;
			uniform float time;
			in vec2 uv;

			out vec4 outcolor;

			void main() {
				float depth0 = texture2D(depthTex, uv).r;

				// mat4 invVP = inverse(projection*view);
				vec4 cpw = inverse(projection) * vec4(uv*2 - 1, depth0*2-1, 1);
				cpw /= cpw.w;
				vec4 wpw = inverse(view) * cpw;
				// vec4 oldwp = inverse(prevProjView) * cpw;

				// vec3 wsvel = vec3(sin(time+wpw.y*0.1),cos(time*3+wpw.y+wpw.z),0) * cpw.z*0.02; //wpw.xyz - oldwp.xyz;
				// if(length(wsvel) > 1.f) wsvel = normalize(wsvel);

				// vec3 csvel = (view * vec4(wsvel*0.1, 0)).xyz;
				// vec4 uvvel = projection * vec4(csvel, 0);
				// uvvel /= uvvel.w;

				// outcolor.rgb = wsvel*.5 + .5;
				// outcolor.a = 1.f;
				// return;
				vec3 wp = wpw.xyz;

				float dd = 2.f;
				vec2 texSize = dd/textureSize(depthTex, 0);
				vec2 uv0 = uv; // + uvvel.xy*0.1f;
				vec2 uv1 = uv0+vec2(texSize.x,0);
				vec2 uv2 = uv0+vec2(0,texSize.y);
				// depth0 = texture2D(depthTex, uv0).r;
				// float depth1 = texture2D(depthTex, uv1).r;
				// float depth2 = texture2D(depthTex, uv2).r;

				mat4 invProj = inverse(projection);
				vec4 cpw0 = invProj * vec4(uv0*2 - 1, depth0*2-1, 1);
				// vec4 cpw1 = invProj * vec4(uv1*2 - 1, depth1*2-1, 1);
				// vec4 cpw2 = invProj * vec4(uv2*2 - 1, depth2*2-1, 1);
				vec3 cp0 = cpw0.xyz/cpw0.w;
				// vec3 cp1 = cpw1.xyz/cpw1.w;
				// vec3 cp2 = cpw2.xyz/cpw2.w;

				vec4 color = texture2D(colorTex, uv0);
				vec4 particle = texture2D(general0Tex, uv0); //*length(cp0);

				float fogdepth = length(cp0); // Distance from eye
				float fogmix = 1-clamp(pow(fogdepth / fogDistance, fogColor.a), 0, 1);

				outcolor.rgb = clamp(color.rgb*fogmix + fogColor.rgb*(1-fogmix), 0, 1);
				outcolor.rgb += particle.rgb * particle.a;
				outcolor.a = 1;
				return;
				// if(normLen - fogdepth/100.f > 0.1f) outcolor.rgb += 0.1f;

				// vec3 norm = cross(cp1-cp0, cp2-cp0);
				// float normLen = length(norm);
				// norm = normalize(mat3(inverse(view)) * norm);

				vec3 lightDiff = vec3(0,3,-210) - wp;
				float dist = length(lightDiff);
				float minDist = time*0.f + 20.f
					+ cos(atan(lightDiff.z, lightDiff.x) * (3.f + sin(time)*2.f + time/10.f) + time) * 3.f;

				if(dist < minDist){
					// outcolor.rgb += (norm*.5+.5)*0.4f;
					// outcolor.rgb += mod(wp/10.f, 1.f);
					outcolor.rgb += sin((vec3(time*2.f, time*3.f, time*5.f)+lightDiff*0.3)/7.f)*.5+.5;
					// outcolor.rgb = vec3(1)-outcolor.rgb;
					// outcolor.rgb = vec3(0);
					
					// if(abs(wp.x) < 0.25f && norm.y > 0.6 || mod(wp.z+0.5f, 30.f) < 0.5f)
					// 	outcolor.rgb = vec3(1.f,0,0);
				}else if(dist < minDist + 20) {
					outcolor.rgb = vec3(minDist + 20 - dist)/20.f;
				}else if(dist < minDist + 90) {
					outcolor.rgb = vec3(0);
				}else if(dist < minDist + 100) {
					outcolor.rgb = mix(outcolor.rgb, vec3(0), (minDist+100-dist)/10.f);
				}

				// outcolor.rgb += csvel;

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

	// Framebuffer antialiasFbo;
}

bool InitEffects() {
	shaderProgram = CreateNamedShaderProgram(ShaderIDPost, postShaderSrc[0], postShaderSrc[1]);
	initialState.fogColor = vec3{0.4, 0.0, 0.2}*0.2f;
	initialState.fogDistance = 30.f;
	initialState.fogDensity = 0.2f;

	initialState.fogColor = vec3{0.1};
	initialState.fogDistance = 200.f;
	initialState.fogDensity = 0.5f;

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
	glUniformMatrix4fv(shaderProgram->projectionLoc, 1, false, glm::value_ptr(camera->projection));
	glUniformMatrix4fv(shaderProgram->viewLoc, 1, false, glm::value_ptr(camera->view));

	u32 timeLoc = glGetUniformLocation(shaderProgram->program, "time");
	u32 fogColorLoc = glGetUniformLocation(shaderProgram->program, "fogColor");
	u32 fogDistanceLoc = glGetUniformLocation(shaderProgram->program, "fogDistance");
	u32 prevProjViewLoc = glGetUniformLocation(shaderProgram->program, "prevProjView");
	glUniform4fv(fogColorLoc, 1, glm::value_ptr(vec4{actualState.fogColor, actualState.fogDensity}));
	glUniform1f(fogDistanceLoc, actualState.fogDistance);

	static mat4 prevView = camera->view;
	glUniformMatrix4fv(prevProjViewLoc, 1, false, glm::value_ptr(/*camera->projection * */prevView));
	prevView = camera->view;

	static f32 time = 0.f;
	glUniform1f(timeLoc, time += dt);

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