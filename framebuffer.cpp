#include "voi.h"

Framebuffer InitFramebuffer(u32 width, u32 height) {
	static u32 fbTargetTypes[] {GL_DEPTH24_STENCIL8, GL_RGB8, GL_RGBA8};
	static u32 fbTargetFormats[] {GL_DEPTH_STENCIL, GL_RGB, GL_RGBA};
	static u32 fbTargetAttach[] {GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	static u32 fbTargetIntType[] {GL_UNSIGNED_INT_24_8, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};

	Framebuffer fb;
	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
	glDrawBuffers(2, &fbTargetAttach[1]);

	glGenTextures(3, fb.targets);
	for(u8 i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, fb.targets[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, fbTargetTypes[i], width, height, 0, fbTargetFormats[i], fbTargetIntType[i], nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, fbTargetAttach[i], GL_TEXTURE_2D, fb.targets[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		puts("Warning! Framebuffer incomplete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
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