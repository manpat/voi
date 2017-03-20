#include "voi.h"
#include "ext/stb_image.h"

#include <algorithm>

namespace {
	ShaderProgram shaderPrograms[256];
}

Framebuffer CreateFramebuffer(FramebufferSettings settings) {
	Framebuffer fb;
	fb.width = settings.width;
	fb.height = settings.height;
	fb.targetCount = settings.numColorBuffers;

	if(settings.hasStencil && !settings.hasDepth) {
		// NOTE: This could be fine, but I cbf googling for stencil only formats 
		LogError("Warning! Tried to create a framebuffer with stencil but no depth\n");
		settings.hasDepth = true;
	}

	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	if(settings.hasDepth) {
		glGenTextures(1, &fb.depthStencilTarget);
		glBindTexture(GL_TEXTURE_2D, fb.depthStencilTarget);
		
		if(settings.hasStencil) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, fb.width, fb.height, 0, 
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		}else{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, fb.width, fb.height, 0, 
				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);			
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, settings.hasStencil?GL_DEPTH_STENCIL_ATTACHMENT:GL_DEPTH_ATTACHMENT, 
			GL_TEXTURE_2D, fb.depthStencilTarget, 0);
	}

	const u32 internalFormat = settings.hdrColorBuffers? GL_RGBA16F : GL_RGBA8;
	const u32 formatType = settings.hdrColorBuffers? GL_FLOAT : GL_UNSIGNED_BYTE;

	glGenTextures(settings.numColorBuffers, fb.colorTargets);
	for(u32 i = 0; i < settings.numColorBuffers; i++) {
		glBindTexture(GL_TEXTURE_2D, fb.colorTargets[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, fb.width, fb.height, 0, 
			GL_RGBA, formatType, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, fb.colorTargets[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	static u32 targetIDs[8] {0,1,2,3,4,5,6,7};
	EnableTargets(settings.numColorBuffers, targetIDs);

	fb.valid = true;
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LogError("Warning! Framebuffer incomplete!\n");
		fb.valid = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
}

void EnableTargets(std::initializer_list<u32> cb) {
	EnableTargets(cb.size(), cb.begin());
}

void EnableTargets(u32 count, const u32* const begin) {
	u32 tmp[8] {0};

	std::copy(begin, begin+std::min(count, 8u), tmp);
	std::transform(tmp, tmp + count, tmp, [](u32 i) {
		return GL_COLOR_ATTACHMENT0 + i;
	});

	glDrawBuffers(count, tmp);
}

void DestroyFramebuffer(Framebuffer* fb) {
	glDeleteTextures(1, &fb->depthStencilTarget);
	glDeleteTextures(fb->targetCount, fb->colorTargets);
	glDeleteFramebuffers(1, &fb->fbo);
	fb->fbo = 0u;
}

static u32 CreateShader(const char* src, u32 type) {
	u32 id = glCreateShader(type);

	const char* shaderSrc[] {
		"#version 130\n",
		src
	};

	glShaderSource(id, GetArraySize(shaderSrc), shaderSrc, nullptr);
	glCompileShader(id);

	s32 status = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if(!status) {
		s32 logLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetShaderInfoLog(id, logLength, nullptr, infoLog);

		LogError("%s\n", infoLog);
		delete[] infoLog;

		glDeleteShader(id);
		return 0;
	}

	return id;
}

ShaderProgram CreateShaderProgram(const char* vsrc, const char* fsrc) {
	ShaderProgram ret{};

	u32 vsh = CreateShader(vsrc, GL_VERTEX_SHADER);
	u32 fsh = CreateShader(fsrc, GL_FRAGMENT_SHADER);

	if(!vsh || !fsh) {
		LogError("Shader compilation failed\n");
		goto error;
	}

	ret.program = glCreateProgram();
	glAttachShader(ret.program, vsh);
	glAttachShader(ret.program, fsh);

	glBindAttribLocation(ret.program, 0, "vertex");
	glBindAttribLocation(ret.program, 1, "uv");

	glBindFragDataLocation(ret.program, 0, "outcolor");
	glBindFragDataLocation(ret.program, 1, "outgeneral0");
	glBindFragDataLocation(ret.program, 2, "outgeneral1");

	glLinkProgram(ret.program);

	s32 linkStatus;
	glGetProgramiv(ret.program, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		s32 logLength = 0;
		glGetProgramiv(ret.program, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetProgramInfoLog(ret.program, logLength, nullptr, infoLog);

		LogError("%s\n", infoLog);
		delete[] infoLog;

		glDeleteProgram(ret.program);
		ret.program = 0;
	}else{
		ret.viewProjectionLoc = glGetUniformLocation(ret.program, "viewProjection");
		ret.projectionLoc = glGetUniformLocation(ret.program, "projection");
		ret.viewLoc = glGetUniformLocation(ret.program, "view");
		ret.modelLoc = glGetUniformLocation(ret.program, "model");

		ret.materialColorLoc = glGetUniformLocation(ret.program, "materialColor");
		ret.clipPlaneLoc = glGetUniformLocation(ret.program, "clipPlane");

		ret.depthTexLoc = glGetUniformLocation(ret.program, "depthTex");
		ret.colorTexLoc = glGetUniformLocation(ret.program, "colorTex");
		ret.general0TexLoc = glGetUniformLocation(ret.program, "general0Tex");
		ret.general1TexLoc = glGetUniformLocation(ret.program, "general1Tex");
		ret.ditherTexLoc = glGetUniformLocation(ret.program, "ditherTex");
	}

error:
	glDeleteShader(vsh);
	glDeleteShader(fsh);
	return ret;
}

ShaderProgram* CreateNamedShaderProgram(u32 shId, const char* vs, const char* fs) {
	assert(shId < 255);
	
	auto prog = &shaderPrograms[shId];
	if(prog->program) {
		LogError("Warning! Overriding already initialised shader program (%u)!\n", shId);
	}

	*prog = CreateShaderProgram(vs, fs);
	return prog;
}

ShaderProgram* GetNamedShaderProgram(u32 shId) {
	assert(shId < 255);
	
	auto prog = &shaderPrograms[shId];
	if(!prog->program) {
		LogError("Warning! Tried to get uninitialised named shader program (%u)!\n", shId);
		return nullptr;
	}
	
	return prog;
}

void UseShaderProgram(const ShaderProgram* sh) {
	assert(sh && sh->program);
	glUseProgram(sh->program);
}

u32 LoadTexture(const char* fname) {
	s32 texWidth, texHeight, numComponents;
	u8* texData = stbi_load(fname, &texWidth, &texHeight, &numComponents, STBI_default);
	if(!texData) {
		LogError("Warning! Unable to load texture \"%s\"\n", fname);
		return 0u;
	}
	
	struct { u32 i, e; } formatMap[] = {
		[STBI_default]		= {0,0}, // Invalid
		[STBI_grey] 		= {GL_R8, GL_RED},
		[STBI_grey_alpha] 	= {GL_RG8, GL_RG}, // NOTE: This isn't quite right, but it's unlikely we'll ever use this
		[STBI_rgb] 			= {GL_RGB8, GL_RGB},
		[STBI_rgb_alpha] 	= {GL_RGBA8, GL_RGBA},
	};

	u32 tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, formatMap[numComponents].i, texWidth, texHeight, 0, 
		formatMap[numComponents].e, GL_UNSIGNED_BYTE, texData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texData);
	return tex;
}

void DrawUnitQuad() {
	static u32 vbo = 0;
	if(!vbo) {
		vec2 verts[] = {
			vec2{-1,-1},
			vec2{ 1,-1},
			vec2{ 1, 1},
			vec2{-1, 1},
		};

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, nullptr);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DrawQuadAtFarPlane(const mat4& projection) {
	static u32 vbo = 0;
	if(!vbo) {
		glGenBuffers(1, &vbo);
	}

	constexpr f32 epsilon = 1e-9;
	mat4 invProj = glm::inverse(projection);
	auto cp0 = invProj * vec4{-1,-1, 1 - epsilon, 1};
	auto cp1 = invProj * vec4{ 1,-1, 1 - epsilon, 1};
	auto cp2 = invProj * vec4{ 1, 1, 1 - epsilon, 1};
	auto cp3 = invProj * vec4{-1, 1, 1 - epsilon, 1};

	vec3 points[] = {
		vec3{cp0/cp0.w},
		vec3{cp1/cp1.w},
		vec3{cp2/cp2.w},
		vec3{cp3/cp3.w},
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

vec3 RGBToHSV(vec3 rgb) {
	auto hsv = glm::hsvColor(rgb);

	hsv.r = std::fmod(hsv.r/360.f, 1.f);
	if(hsv.r < 0.f)
		hsv.r = 1.f + hsv.r;

	return hsv;
}

vec3 HSVToRGB(vec3 hsv) {
	hsv.r = std::fmod(hsv.r*360.f, 360.f);
	if(hsv.r < 0.f)
		hsv.r = 360.f + hsv.r;

	return glm::rgbColor(hsv);
}