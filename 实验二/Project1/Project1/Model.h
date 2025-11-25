#pragma once
#include<vector>
#include<GL/glew.h>
#include<string>
#include "Vertex.h"
class Model
{
public:

	bool LoadModelFile(const std::string& filename);

	void uploadToGPU();

	void draw() const;

private:
	void normalizeToUnitBox(); //坐标归一化

	std::vector<Vertex> m_vertices;
	bool m_uploaded;
	GLuint m_vbo;
};

