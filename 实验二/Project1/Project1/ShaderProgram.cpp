#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram() : m_program(0) {}

ShaderProgram::~ShaderProgram() {
	if (m_program != 0) {
		glDeleteProgram(m_program);
	}
}

GLuint ShaderProgram::compile(GLenum type, const std::string& src) {
	GLuint shader = glCreateShader(type);
	const char* cstr = src.c_str();
	glShaderSource(shader, 1, &cstr, nullptr);
	glCompileShader(shader);

	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, log);
		std::cerr << "Shader compile error: " << log << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

bool ShaderProgram::createFromSource(const std::string& vsSrc, const std::string& fsSrc) {
	//编译顶点、片段着色器
	GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
	GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);

	m_program = glCreateProgram();
	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);
	glBindAttribLocation(m_program, 0, "aPos");
	glBindAttribLocation(m_program, 1, "aColor");

	glLinkProgram(m_program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	GLint linked = 0;
	glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		char log[1024];
		glGetProgramInfoLog(m_program, 1024, nullptr, log);
		std::cerr << "Program link error: " << log << std::endl;
		glDeleteProgram(m_program);
		m_program = 0;
		return false;
	}
	return true;
}

void ShaderProgram::use() const {
	glUseProgram(m_program);
}

GLuint ShaderProgram::id() const {
	return m_program; 
}

GLint ShaderProgram::getUniformLocation(const char* name) const{
	return glGetUniformLocation(m_program, name);
}