
#include "Model.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cfloat>
#include "vec.h" 

Model::Model() : m_vbo(0), m_uploaded(false) {}

Model::~Model() {
	if (m_vbo != 0) {
		glDeleteBuffers(1, &m_vbo);
	}
}

bool Model::LoadModelFile(const std::string& filename) {
	std::ifstream inputfile(filename);
	if (!inputfile) {
		std::cout << "Fail to load obj file " << filename << std::endl;
		return false;
	}

	std::vector<Vec3> positions;
	std::vector<Vec3> colors;
	std::vector<Vertex> tmp_vertices;

	std::string line;
	while (std::getline(inputfile, line)) {
		if (line.empty()) {
			continue;
		}
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

				if (i_0 < 0 || i_1 < 0 || i_2 < 0 ||
					(size_t)i_0 >= positions.size() ||
					(size_t)i_1 >= positions.size() ||
					(size_t)i_2 >= positions.size()) {
					continue;
				}

				Vertex v_0, v_1, v_2;
				v_0.pos = positions[i_0];
				v_1.pos = positions[i_1];
				v_2.pos = positions[i_2];

				v_0.color = (i_0 < colors.size()) ? colors[i_0] : Vec3(1.0f, 1.0f, 1.0f);
				v_1.color = (i_1 < colors.size()) ? colors[i_1] : Vec3(1.0f, 1.0f, 1.0f);
				v_2.color = (i_2 < colors.size()) ? colors[i_2] : Vec3(1.0f, 1.0f, 1.0f);

				tmp_vertices.push_back(v_0);
				tmp_vertices.push_back(v_1);
				tmp_vertices.push_back(v_2);
			}
		}
	}

	if (positions.empty() || tmp_vertices.empty())
	{
		std::cerr << "No valid geometry in obj.\n";
		return false;
	}

	m_vertices = std::move(tmp_vertices);
	normalizeToUnitBox();

	if (m_vbo != 0) {
		glDeleteBuffers(1, &m_vbo);
		m_vbo = 0;
	}
	m_uploaded = false;

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
		return;
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

bool Model::isEmpty() const {
	return m_vertices.empty();
}