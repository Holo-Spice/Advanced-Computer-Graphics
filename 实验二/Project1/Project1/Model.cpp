#include "Model.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "vec.h" 
bool Model::LoadModelFile(const std::string& filename) {
	std::ifstream inputfile(filename);
	if (!inputfile) {
		std::cout << "Fail to load obj file" << filename << std::endl;
		return false;
	}
	// 顶点位置 顶点颜色
	std::vector<Vec3> positions;
	std::vector<Vec3> colors;

	std::string line;
	while (std::getline(inputfile, line)) {
		if (line.empty()) {
			continue;
		}
		// 读取行首字符
		std::stringstream ss(line);
		std::string tag;
		ss >> tag;

		if (tag == "v") {
			float x, y, z;
			ss >> x >> y >> z;
			positions.emplace_back(x, y, z);
		}
		else if (tag == "vc") {
			float r, g, b;
			ss >> r >> g >> b;
			colors.emplace_back(r, g, b);
		}
		else if (tag == "f") {
			std::vector<int> indices;
			int idx;
			while (ss >> idx)
			{
				indices.push_back(idx);
			}
			if (indices.size() < 3) {
				continue;
			}
			// 三角扇拆面：(0, i, i+1)
			for (size_t i = 1; i + 1 < indices.size(); i++) {
				int i_0 = indices[0] - 1;
				int i_1 = indices[i] - 1;
				int i_2 = indices[i + 1] - 1;

				Vertex v_0, v_1, v_2;
				v_0.pos = positions[i_0];
				v_1.pos = positions[i_1];
				v_2.pos = positions[i_2];
				v_0.color = colors[i_0];
				v_1.color = colors[i_1];
				v_2.color = colors[i_2];

				m_vertices.push_back(v_0);
				m_vertices.push_back(v_1);
				m_vertices.push_back(v_2);
			}
		}
	}
	if (positions.empty() || m_vertices.empty())
	{
		std::cerr << "No valid geometry in obj.\n";
		return false;
	}

	normalizeToUnitBox();

	std::cout << "OBJ loaded: " << m_vertices.size() << " vertices\n";
	return true;
}
void Model::normalizeToUnitBox() {
	Vec3 bmin(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3 bmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const auto& v : m_vertices)
	{
		bmin.x = std::min(bmin.x, v.pos.x);
		bmin.y = std::min(bmin.y, v.pos.y);
		bmin.z = std::min(bmin.z, v.pos.z);
		bmax.x = std::max(bmax.x, v.pos.x);
		bmax.y = std::max(bmax.y, v.pos.y);
		bmax.z = std::max(bmax.z, v.pos.z);
	}

	Vec3 center((bmin.x + bmax.x) * 0.5f,
		(bmin.y + bmax.y) * 0.5f,
		(bmin.z + bmax.z) * 0.5f);

	Vec3 extent(bmax.x - bmin.x,
		bmax.y - bmin.y,
		bmax.z - bmin.z);

	float maxExtent = std::max(extent.x, std::max(extent.y, extent.z));
	if (maxExtent < 1e-6f) maxExtent = 1.0f;

	float scale = 2.0f / maxExtent;

	for (auto& v : m_vertices)
	{
		v.pos = (v.pos - center) * scale;
	}
}

void Model::uploadToGPU() {
	if (m_uploaded || m_vertices.empty()) {
		return ;
	}

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 m_vertices.size() * sizeof(Vertex),
				 m_vertices.data(),
				 GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	m_uploaded = true;
}

void Model::draw() const {
	if (!m_uploaded || m_vbo == 0) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,                    
						  3,                    
						  GL_FLOAT,              
						  GL_FALSE,              
						  sizeof(Vertex),         
						 (const GLvoid*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,                      
						  3,                     
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						 (const GLvoid*)offsetof(Vertex, color));

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}