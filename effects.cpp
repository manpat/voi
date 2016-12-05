#include "voi.h"

namespace {
	#define SHADER(x) "#version 130\n" #x
	const char* vertSrc = SHADER(
		in vec2 vertex;
		out vec2 uv;

		void main() {
			gl_Position = vec4(vertex, 0, 1);
			uv = vertex * 0.5f + 0.5f;
		}
	);

	const char* postShaderSrc = SHADER(
		uniform sampler2D depthTex;
		uniform sampler2D colorTex;
		uniform sampler2D general0Tex;
		uniform sampler2D general1Tex;
		uniform sampler2D ditherTex;
		uniform mat4 invProjection;
		uniform mat4 prevProjView;
		uniform mat4 projection;
		uniform mat4 view;
		uniform vec4 fogColor;
		uniform float fogDistance;
		uniform float vignettePower;
		uniform float vignetteStrength;
		uniform float time;
		in vec2 uv;

		out vec4 outcolor;

		void main() {
			float depth0 = texture2D(depthTex, uv).r;
			float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;

			// mat4 invVP = inverse(projection*view);
			// vec4 cpw = inverse(projection) * vec4(uv*2 - 1, depth0*2-1, 1);
			// cpw /= cpw.w;
			// vec4 wpw = inverse(view) * cpw;
			// vec4 oldwp = inverse(prevProjView) * cpw;

			// vec3 wsvel = vec3(sin(time+wpw.y*0.1),cos(time*3+wpw.y+wpw.z),0) * cpw.z*0.02; //wpw.xyz - oldwp.xyz;
			// if(length(wsvel) > 1.f) wsvel = normalize(wsvel);

			// vec3 csvel = (view * vec4(wsvel*0.1, 0)).xyz;
			// vec4 uvvel = projection * vec4(csvel, 0);
			// uvvel /= uvvel.w;

			// outcolor.rgb = wsvel*.5 + .5;
			// outcolor.a = 1.f;
			// return;
			// vec3 wp = wpw.xyz;

			// float dd = 2.f;
			// vec2 texSize = dd/textureSize(depthTex, 0);
			vec2 uv0 = uv; // + uvvel.xy*0.1f;
			// vec2 uv1 = uv0+vec2(texSize.x,0);
			// vec2 uv2 = uv0+vec2(0,texSize.y);
			// depth0 = texture2D(depthTex, uv0).r;
			// float depth1 = texture2D(depthTex, uv1).r;
			// float depth2 = texture2D(depthTex, uv2).r;

			vec4 cpw0 = invProjection * vec4(uv0*2 - 1, depth0*2-1, 1);
			// vec4 cpw1 = invProj * vec4(uv1*2 - 1, depth1*2-1, 1);
			// vec4 cpw2 = invProj * vec4(uv2*2 - 1, depth2*2-1, 1);
			vec3 cp0 = cpw0.xyz/cpw0.w;
			// vec3 cp1 = cpw1.xyz/cpw1.w;
			// vec3 cp2 = cpw2.xyz/cpw2.w;

			vec4 color = texture2D(colorTex, uv0);
			vec4 particle = texture2D(general0Tex, uv0);

			float fogdepth = length(cp0) + dither; // Distance from eye
			float fogmix = 1-clamp(pow(fogdepth / fogDistance, fogColor.a), 0, 1) * color.a;

			vec4 bloom = texture2D(general1Tex, uv);
			bloom += textureLod(general1Tex, uv, 1);
			bloom += textureLod(general1Tex, uv, 2);
			bloom += textureLod(general1Tex, uv, 3);
			bloom += textureLod(general1Tex, uv, 4);
			bloom += textureLod(general1Tex, uv, 5);
			// outcolor = bloom;
			// outcolor.a = 1.f;
			// return;

			outcolor.rgb = clamp(color.rgb*fogmix + fogColor.rgb*(1-fogmix), 0, 1);
			outcolor.rgb += particle.rgb * particle.a;
			outcolor.rgb += bloom.rgb * 0.05;
			
			float vignette = clamp(1 - pow(length(uv-.5), vignettePower) * vignetteStrength, 0.3, 1);
			outcolor.rgb *= vignette + dither;
			outcolor.a = color.a;
			return;
			// if(normLen - fogdepth/100.f > 0.1f) outcolor.rgb += 0.1f;

			// vec3 norm = cross(cp1-cp0, cp2-cp0);
			// float normLen = length(norm);
			// norm = normalize(mat3(inverse(view)) * norm);

			/*
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
			*/

			// outcolor.rgb += csvel;

			// float step = 16.f;
			// outcolor.rgb = floor(outcolor.rgb*step)/step;
		}
	);

	const char* bloomFragSrc = SHADER(
		uniform sampler2D colorTex;
		uniform sampler2D ditherTex;
		uniform vec2 step;

		in vec2 uv;
		out vec4 outcolor;

		vec4 sample(vec2 p, float v) {
			return texture2D(colorTex, p+step*v);
		}

		void main() {
			float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;
			// float sum = 256.f;
			// vec4 accum = sample(uv, 0) * 70.f/sum;
			// accum += sample(uv, 1) * 56.f/sum;
			// accum += sample(uv,-1) * 56.f/sum;
			// accum += sample(uv, 2) * 28.f/sum;
			// accum += sample(uv,-2) * 28.f/sum;
			// accum += sample(uv, 3) * 8.f/sum;
			// accum += sample(uv,-3) * 8.f/sum;
			// accum += sample(uv, 4) * 1.f/sum;
			// accum += sample(uv,-4) * 1.f/sum;
			float sum = 16.f;
			vec4 accum = sample(uv, 0) * 6.f/sum;
			accum += sample(uv, 1) * 4.f/sum;
			accum += sample(uv,-1) * 4.f/sum;
			accum += sample(uv, 2) * 1.f/sum;
			accum += sample(uv,-2) * 1.f/sum;
			accum.a = clamp(accum.a, 0, 1);

			outcolor = accum;
			outcolor.rgb += vec3(dither);
		}
	);

	ShaderProgram* shaderProgram;
	ShaderProgram bloomProgram;

	template<class T>
	struct Lerpable {
		T initial;
		T target;
		T actual;
		f32 lerpAmt;
		f32 lerpTime = 4.f;

		void operator=(T o) {
			initial = target = actual = o;
		}

		operator T() {
			return actual;
		}

		void Update(f32 dt) {
			lerpAmt = glm::clamp(lerpAmt+dt/lerpTime, 0.f, 1.f);
			f32 term = lerpAmt*lerpAmt*(3.f - 2.f*lerpAmt); // Smoothstep
			actual = glm::mix(initial, target, term);
		}

		void SetTarget(T o) {
			target = o;
			initial = actual;
			lerpAmt = 0.f;
		}

		void SetDuration(f32 o) {
			lerpTime = std::max(o, 0.0001f);
		}
	};

	Lerpable<vec3> fogColor;
	Lerpable<f32> fogDistance;
	Lerpable<f32> fogDensity;

	Lerpable<f32> vignetteLevel;

	u32 ditherTex;

	Framebuffer secondaryFbo;
	// Framebuffer tertiaryFbo;
}

extern u32 windowWidth, windowHeight;

bool InitEffects() {
	shaderProgram = CreateNamedShaderProgram(ShaderIDPost, vertSrc, postShaderSrc);
	bloomProgram = CreateShaderProgram(vertSrc, bloomFragSrc);
	fogColor = vec3{0.1};
	fogDistance = 200.f;
	fogDensity = 0.5f;

	vignetteLevel = 0.1f;

	return ReinitEffects();
}

bool ReinitEffects() {
	u8* ditherPattern = new u8[windowWidth*windowHeight];
	for(u32 i = 0; i < windowWidth*windowHeight; i++)
		ditherPattern[i] = (std::rand()&7)<<5;

	if(ditherTex) glDeleteTextures(1, &ditherTex);
	glGenTextures(1, &ditherTex);
	glBindTexture(GL_TEXTURE_2D, ditherTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, windowWidth, windowHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ditherPattern);
	glBindTexture(GL_TEXTURE_2D, 0);

	FramebufferSettings settings {
		.width = windowWidth,
		.height = windowHeight,
		.numColorBuffers = 1,
		.hasStencil = false,
		.hasDepth = false,
		.filter = false
	};

	DestroyFramebuffer(&secondaryFbo);
	secondaryFbo = CreateFramebuffer(settings);
	if(!secondaryFbo.valid) {
		LogError("Effect framebuffer init failed\n");
		return false;
	}

	// DestroyFramebuffer(&tertiaryFbo);
	// tertiaryFbo = CreateFramebuffer(settings);
	// if(!tertiaryFbo.valid) {
	// 	LogError("Effect framebuffer init failed\n");
	// 	return false;
	// }

	delete[] ditherPattern;
	return true;
}

void ApplyEffectsAndDraw(Framebuffer* fb, const Camera* camera, f32 dt) {
	fogColor.Update(dt);
	fogDensity.Update(dt);
	fogDistance.Update(dt);
	vignetteLevel.Update(dt);

	glUseProgram(bloomProgram.program);
	u32 stepLoc = glGetUniformLocation(bloomProgram.program, "step");

	// Apply bloom
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
	EnableTargets({2});
	glActiveTextureVoi(GL_TEXTURE0);
	glUniform1i(bloomProgram.colorTexLoc, 0);
	// constexpr f32 stepsize[] = {11.f, 5.f, 2.f};
	// constexpr f32 stepsize[] = {};
	// constexpr f32 stepsize[] = {/*31.f, */11.f, 5.f};
	// constexpr f32 stepsize[] = {100.f, 31.f, 11.f, 5.f};
	// constexpr f32 stepsize[] = {101.f/glm::sqrt(2.f), 41.f/glm::sqrt(2.f), 17.f/glm::sqrt(2.f), 2.f/glm::sqrt(2.f)};

	// for(u32 i = 0; i < sizeof(stepsize)/sizeof(f32); i++) {
	// 	glBindFramebuffer(GL_FRAMEBUFFER, secondaryFbo.fbo);
	// 	// glUniform2f(stepLoc, 0.f, stepsize[i]/fb->height);
	// 	glUniform2f(stepLoc, stepsize[i]/fb->width, stepsize[i]/fb->height);
	// 	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[2]);
	// 	glClear(GL_COLOR_BUFFER_BIT);
	// 	DrawFullscreenQuad();
		
	// 	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
	// 	// glUniform2f(stepLoc, stepsize[i]/secondaryFbo.width, 0.f);
	// 	glUniform2f(stepLoc, stepsize[i]/secondaryFbo.width,-stepsize[i]/secondaryFbo.height);
	// 	glBindTexture(GL_TEXTURE_2D, secondaryFbo.colorTargets[0]);
	// 	glClear(GL_COLOR_BUFFER_BIT);
	// 	DrawFullscreenQuad();
	// }
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(shaderProgram->program);

	// Apply fog and various effects
	glActiveTextureVoi(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb->depthStencilTarget);
	glActiveTextureVoi(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[0]);
	glActiveTextureVoi(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[1]);
	glActiveTextureVoi(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[2]);
	glActiveTextureVoi(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ditherTex);

	glUniform1i(shaderProgram->depthTexLoc, 0);
	glUniform1i(shaderProgram->colorTexLoc, 1);
	glUniform1i(shaderProgram->general0TexLoc, 2);
	glUniform1i(shaderProgram->general1TexLoc, 3);
	glUniform1i(shaderProgram->ditherTexLoc, 4);
	glUniformMatrix4fv(shaderProgram->projectionLoc, 1, false, glm::value_ptr(camera->projection));
	glUniformMatrix4fv(shaderProgram->viewLoc, 1, false, glm::value_ptr(camera->view));
	u32 invProjectionLoc = glGetUniformLocation(shaderProgram->program, "invProjection");
	glUniformMatrix4fv(invProjectionLoc, 1, false, glm::value_ptr(glm::inverse(camera->projection)));

	u32 timeLoc = glGetUniformLocation(shaderProgram->program, "time");
	u32 fogColorLoc = glGetUniformLocation(shaderProgram->program, "fogColor");
	u32 fogDistanceLoc = glGetUniformLocation(shaderProgram->program, "fogDistance");
	u32 prevProjViewLoc = glGetUniformLocation(shaderProgram->program, "prevProjView");
	u32 vignettePowerLoc = glGetUniformLocation(shaderProgram->program, "vignettePower");
	u32 vignetteStrengthLoc = glGetUniformLocation(shaderProgram->program, "vignetteStrength");
	glUniform4fv(fogColorLoc, 1, glm::value_ptr(vec4{fogColor.actual, fogDensity.actual}));
	glUniform1f(fogDistanceLoc, fogDistance.actual);

	static mat4 prevView = camera->view;
	glUniformMatrix4fv(prevProjViewLoc, 1, false, glm::value_ptr(/*camera->projection * */prevView));
	prevView = camera->view;

	static f32 time = 0.f;
	glUniform1f(timeLoc, time += dt);

	glUniform1f(vignettePowerLoc, 3.5f - vignetteLevel*1.2f);
	glUniform1f(vignetteStrengthLoc, 0.1f + vignetteLevel * 1.2f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	DrawFullscreenQuad();

	// TODO: antialiasing
	//http://www.gamedev.net/topic/580517-nfaa---a-post-process-anti-aliasing-filter-results-implementation-details/
}

void SetTargetFogParameters(const vec3& color, f32 distance, f32 density, f32 duration) {
	fogColor.SetTarget(color);
	fogDensity.SetTarget(density);
	fogDistance.SetTarget(distance);
	fogColor.SetDuration(duration);
	fogDensity.SetDuration(duration);
	fogDistance.SetDuration(duration);
}

void SetTargetFogColor(const vec3& color, f32 duration) {
	fogColor.SetTarget(color);
	fogColor.SetDuration(duration);
}

void SetTargetFogDensity(f32 density, f32 duration) {
	fogDensity.SetTarget(density);
	fogDensity.SetDuration(duration);
}

void SetTargetFogDistance(f32 distance, f32 duration) {
	fogDistance.SetTarget(distance);
	fogDistance.SetDuration(duration);
}

void SetTargetVignetteLevel(f32 lvl, f32 duration) {
	vignetteLevel.SetTarget(lvl);
	vignetteLevel.SetDuration(duration);
}
