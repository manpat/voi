#include "voi.h"

// https://docs.unrealengine.com/udk/Three/Bloom.html
// https://learnopengl.com/#!Advanced-Lighting/HDR

namespace {
	#define SHADER(x) #x
	const char* vertSrc = SHADER(
		in vec2 vertex;
		out vec2 uv;

		void main() {
			gl_Position = vec4(vertex, 0, 1);
			uv = vertex * 0.5f + 0.5f;
		}
	);

	ShaderProgram* finalComposeProgram;
	ShaderProgram thresholdProgram;
	ShaderProgram blurProgram;
	ShaderProgram fogProgram;

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

		void UpdateColor(f32 dt) {
			lerpAmt = glm::clamp(lerpAmt+dt/lerpTime, 0.f, 1.f);
			f32 term = lerpAmt*lerpAmt*(3.f - 2.f*lerpAmt); // Smoothstep
			actual.g = glm::mix(initial.g, target.g, term);
			actual.b = glm::mix(initial.b, target.b, term);
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

	template<>
	struct Lerpable<vec3> {
		vec3 initial;
		vec3 target;
		vec3 actual;
		f32 lerpAmt;
		f32 lerpTime = 4.f;

		void operator=(vec3 o) {
			vec3 hsv = RGBToHSV(o);
			if(std::isnan(hsv.r))
				hsv.r = 0.f;

			initial = target = hsv;
			actual = o;
		}

		operator vec3() {
			return actual;
		}

		void Update(f32 dt) {
			lerpAmt = glm::clamp(lerpAmt+dt/lerpTime, 0.f, 1.f);
			f32 term = lerpAmt*lerpAmt*(3.f - 2.f*lerpAmt); // Smoothstep
			f32 hueDiff = target.r - initial.r;
			vec3 hsv{};

			if(hueDiff*hueDiff > 0.25f) {
				if(hueDiff < 0.f)
					hsv.r = glm::mix(initial.r-1, target.r, term);
				else
					hsv.r = glm::mix(initial.r+1, target.r, term);
			}else{
				hsv.r = glm::mix(initial.r, target.r, term);
			}

			hsv.g = glm::mix(initial.g, target.g, term);
			hsv.b = glm::mix(initial.b, target.b, term);
			actual = HSVToRGB(hsv);
		}

		void SetTarget(vec3 o) {
			f32 r = initial.r;

			target = RGBToHSV(o);
			initial = RGBToHSV(actual);
			if(std::isnan(initial.r))
				initial.r = r;

			if(std::isnan(target.r))
				target.r = initial.r;

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
	Framebuffer blurFBOs[2];
}

extern u32 windowWidth, windowHeight;

bool InitEffects() {
	finalComposeProgram = CreateNamedShaderProgram(ShaderIDPost, vertSrc, LoadFileStatically("data/shaders/post.frag"));
	thresholdProgram = CreateShaderProgram(vertSrc, LoadFileStatically("data/shaders/threshold.frag"));
	blurProgram = CreateShaderProgram(vertSrc, LoadFileStatically("data/shaders/blur.frag"));
	fogProgram = CreateShaderProgram(vertSrc, LoadFileStatically("data/shaders/fog.frag"));
	fogColor = vec3{0.};
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
		.filter = true,
		.hdrColorBuffers = true
	};

	DestroyFramebuffer(&secondaryFbo);
	secondaryFbo = CreateFramebuffer(settings);
	if(!secondaryFbo.valid) {
		LogError("Effect framebuffer init failed\n");
		return false;
	}

	FramebufferSettings blurSettings {
		.width = windowWidth/2,
		.height = windowHeight/2,
		.numColorBuffers = 1,
		.hasStencil = false,
		.hasDepth = false,
		.filter = true,
		.hdrColorBuffers = true
	};

	for(auto& fbo: blurFBOs) {
		DestroyFramebuffer(&fbo);
		fbo = CreateFramebuffer(blurSettings);
		if(!fbo.valid) {
			LogError("Effect framebuffer init failed\n");
			return false;
		}
	}

	delete[] ditherPattern;
	return true;
}

void ApplyEffectsAndDraw(Framebuffer* fb, const Camera* camera, f32 dt) {
	fogColor.Update(dt);
	fogDensity.Update(dt);
	fogDistance.Update(dt);
	vignetteLevel.Update(dt);

	// Apply fog and sky
	glBindFramebuffer(GL_FRAMEBUFFER, secondaryFbo.fbo);
	glViewport(0, 0, secondaryFbo.width, secondaryFbo.height);
	UseShaderProgram(&fogProgram);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTextureVoi(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb->depthStencilTarget);
	glActiveTextureVoi(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[FBTargetColor]);
	glActiveTextureVoi(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ditherTex);

	const u32 fogColorLoc = glGetUniformLocation(fogProgram.program, "fogColor");
	const u32 fogDistanceLoc = glGetUniformLocation(fogProgram.program, "fogDistance");
	const u32 invProjectionLoc = glGetUniformLocation(fogProgram.program, "invProjection");

	glUniform1i(fogProgram.depthTexLoc, 0);
	glUniform1i(fogProgram.colorTexLoc, 1);
	glUniform1i(fogProgram.ditherTexLoc, 2);
	glUniformMatrix4fv(invProjectionLoc, 1, false, glm::value_ptr(glm::inverse(camera->projection)));

	glUniform4fv(fogColorLoc, 1, glm::value_ptr(vec4{fogColor.actual, fogDensity.actual}));
	glUniform1f(fogDistanceLoc, fogDistance.actual);

	DrawUnitQuad();

	// Take threshold
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[0].fbo);
	glViewport(0, 0, blurFBOs[0].width, blurFBOs[0].height);
	UseShaderProgram(&thresholdProgram);
	glActiveTextureVoi(GL_TEXTURE0);
	glUniform1i(thresholdProgram.colorTexLoc, 0);
	glBindTexture(GL_TEXTURE_2D, secondaryFbo.colorTargets[FBTargetColor]);

	DrawUnitQuad();

	// Blur threshold
	UseShaderProgram(&blurProgram);
	u32 stepLoc = glGetUniformLocation(blurProgram.program, "step");
	glUniform1i(blurProgram.colorTexLoc, 0);

	// constexpr f32 stepsize[] = {11.f, 5.f, 2.f};
	// constexpr f32 stepsize[] = {17.f, 5.f, 1.5f};
	// constexpr f32 stepsize[] = {9.f, 5.f, 2.f, 1.f};
	constexpr f32 stepsize[] = {5.f, 2.f, 1.1f, 0.7f};
	// constexpr f32 stepsize[] = {1.0f};
	// constexpr f32 stepsize[] = {31.f, 11.f, 5.f};
	// constexpr f32 stepsize[] = {100.f, 31.f, 11.f, 5.f};
	// constexpr f32 stepsize[] = {101.f/glm::sqrt(2.f), 41.f/glm::sqrt(2.f), 17.f/glm::sqrt(2.f), 2.f/glm::sqrt(2.f)};

	for(u32 i = 0; i < GetArraySize(stepsize); i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[1].fbo);
		glUniform2f(stepLoc, 0.f, stepsize[i]/blurFBOs[0].height);
		glBindTexture(GL_TEXTURE_2D, blurFBOs[0].colorTargets[FBTargetColor]);
		DrawUnitQuad();
		
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[0].fbo);
		glUniform2f(stepLoc, stepsize[i]/blurFBOs[1].width, 0.f);
		glBindTexture(GL_TEXTURE_2D, blurFBOs[1].colorTargets[FBTargetColor]);
		DrawUnitQuad();
	}

	// Compose final image
	UseShaderProgram(finalComposeProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0, windowWidth, windowHeight);

	glActiveTextureVoi(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb->depthStencilTarget);
	glActiveTextureVoi(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, secondaryFbo.colorTargets[FBTargetColor]);
	glActiveTextureVoi(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fb->colorTargets[FBTargetGeneral0]);
	glActiveTextureVoi(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, blurFBOs[0].colorTargets[FBTargetColor]);
	glActiveTextureVoi(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ditherTex);

	glUniform1i(finalComposeProgram->depthTexLoc, 0);
	glUniform1i(finalComposeProgram->colorTexLoc, 1);
	glUniform1i(finalComposeProgram->general0TexLoc, 2);
	glUniform1i(finalComposeProgram->general1TexLoc, 3);
	glUniform1i(finalComposeProgram->ditherTexLoc, 4);
	glUniformMatrix4fv(finalComposeProgram->projectionLoc, 1, false, glm::value_ptr(camera->projection));
	glUniformMatrix4fv(finalComposeProgram->viewLoc, 1, false, glm::value_ptr(camera->view));

	u32 timeLoc = glGetUniformLocation(finalComposeProgram->program, "time");
	u32 vignettePowerLoc = glGetUniformLocation(finalComposeProgram->program, "vignettePower");
	u32 vignetteStrengthLoc = glGetUniformLocation(finalComposeProgram->program, "vignetteStrength");

	static f32 time = 0.f;
	glUniform1f(timeLoc, time += dt);

	glUniform1f(vignettePowerLoc, 3.5f - vignetteLevel*1.2f);
	glUniform1f(vignetteStrengthLoc, 0.1f + vignetteLevel * 1.2f);

	glClear(GL_COLOR_BUFFER_BIT);
	DrawUnitQuad();

	// TODO: antialiasing
	//http://www.gamedev.net/topic/580517-nfaa---a-post-process-anti-aliasing-filter-results-implementation-details/
}

void SetTargetFogParameters(vec3 color, f32 distance, f32 density, f32 duration) {
	fogColor.SetTarget(color);
	fogDensity.SetTarget(density);
	fogDistance.SetTarget(distance);
	fogColor.SetDuration(duration);
	fogDensity.SetDuration(duration);
	fogDistance.SetDuration(duration);
}

void SetTargetFogColor(vec3 color, f32 duration) {
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
