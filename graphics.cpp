#include "voi.h"
#include "ext/stb_image.h"

namespace {
	ShaderProgram shaderPrograms[256];
}

Framebuffer CreateMainFramebuffer(u32 width, u32 height, bool filter) {
	static u32 fbTargetTypes[] {GL_DEPTH24_STENCIL8, GL_RGB8, GL_RGBA8};
	static u32 fbTargetFormats[] {GL_DEPTH_STENCIL, GL_RGB, GL_RGBA};
	static u32 fbTargetAttach[] {GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	static u32 fbTargetIntType[] {GL_UNSIGNED_INT_24_8, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};

	Framebuffer fb;
	fb.width = width;
	fb.height = height;
	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
	glDrawBuffers(2, &fbTargetAttach[1]);

	glGenTextures(3, fb.targets);
	for(u8 i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, fb.targets[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, fbTargetTypes[i], width, height, 0, 
			fbTargetFormats[i], fbTargetIntType[i], nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, fbTargetAttach[i], GL_TEXTURE_2D, fb.targets[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	fb.valid = true;
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		puts("Warning! Framebuffer incomplete!");
		fb.valid = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
}

Framebuffer CreateColorFramebuffer(u32 width, u32 height, bool filter) {
	Framebuffer fb;
	fb.width = width;
	fb.height = height;
	for(u8 i = 0; i < FBTargetCount; i++) fb.targets[i] = 0;

	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	u32 fbTargetAttach[] {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, fbTargetAttach);

	glGenTextures(1, &fb.targets[FBTargetColor]);
	glBindTexture(GL_TEXTURE_2D, fb.targets[FBTargetColor]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.targets[FBTargetColor], 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	fb.valid = true;
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		puts("Warning! Framebuffer incomplete!");
		fb.valid = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
}

void DestroyFramebuffer(Framebuffer* fb) {
	glDeleteTextures(FBTargetCount, fb->targets);
	glDeleteFramebuffers(1, &fb->fbo);
	fb->fbo = 0u;
}

static u32 CreateShader(const char* src, u32 type) {
	u32 id = glCreateShader(type);

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	s32 status = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if(!status) {
		s32 logLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		// Get the info log and print it out
		auto infoLog = new char[logLength];
		glGetShaderInfoLog(id, logLength, nullptr, infoLog);

		fprintf(stderr, "%s\n", infoLog);
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
		fprintf(stderr, "Shader compilation failed\n");
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

		fprintf(stderr, "%s\n", infoLog);
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
		fprintf(stderr, "Warning! Overriding already initialised shader program (%u)!\n", shId);
	}

	*prog = CreateShaderProgram(vs, fs);
	return prog;
}

ShaderProgram* GetNamedShaderProgram(u32 shId) {
	assert(shId < 255);
	
	auto prog = &shaderPrograms[shId];
	if(!prog->program) {
		fprintf(stderr, "Warning! Tried to get uninitialised named shader program (%u)!\n", shId);
		return nullptr;
	}
	
	return prog;
}

u32 LoadTexture(const char* fname) {
	s32 texWidth, texHeight, numComponents;
	u8* texData = stbi_load(fname, &texWidth, &texHeight, &numComponents, STBI_default);
	if(!texData) {
		printf("Warning! Unable to load texture \"%s\"\n", fname);
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

void DrawFullscreenQuad() {
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
