#include "voi.h"

u32 CreateShader(const char* src, u32 type) {
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