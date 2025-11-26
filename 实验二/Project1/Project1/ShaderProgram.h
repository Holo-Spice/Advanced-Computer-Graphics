#pragma once
#include <string>
#include <GL/glew.h>

class ShaderProgram
{
public:
	ShaderProgram(); 
	~ShaderProgram();

	// 从顶点/片段着色器源码创建并链接着色器程序
	bool createFromSource(const std::string& vsSrc, const std::string& fsSrc);
	// 使用着色器
	void use() const;
	// 获取ID
	GLuint id() const;
	// 获取uniform变量位置
	GLint getUniformLocation(const char* name) const;

private:
	// 编译指定类型的着色器
	static GLuint compile(GLenum type, const std::string& src);
	// ID
	GLuint m_program;
};

