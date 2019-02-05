/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INCLUDES & DEFINES////////

#pragma once

#include <vector>
#include <unordered_map>

#include <Vertex.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// GLOBAL VARS ////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// FUNCS ////////

class Mesh {
private:
	std::vector<Vertex>* m_pVertices;
	std::vector<uint32_t>* m_pIndices;
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

public:
	Mesh() {

	}

	void setVertexStore(std::vector<Vertex>* vertices) {
		m_pVertices = vertices;
	}

	void setIndexStore(std::vector<uint32_t>* indices) {
		m_pIndices = indices;
	}

	void create(const char* path) {
		if (!m_pVertices)
		{
			setVertexStore(&m_vertices);
		}
		if (!m_pIndices)
		{
			setIndexStore(&m_indices);
		}
		tinyobj::attrib_t vertexAttributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errorString;
		std::string warnString;

		bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warnString, &errorString, path);

		if (!success)
		{
			throw std::runtime_error(warnString + errorString); 
		}

		std::unordered_map<Vertex, uint32_t> vertices;

		for (tinyobj::shape_t shape : shapes) 
		{
			for (tinyobj::index_t index : shape.mesh.indices) 
			{
				glm::vec3 pos = {
					vertexAttributes.vertices[3 * index.vertex_index + 0],
					vertexAttributes.vertices[3 * index.vertex_index + 2],
					vertexAttributes.vertices[3 * index.vertex_index + 1]
				};

				Vertex vert(pos, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0, 0 }); //see https://youtu.be/KedrqATjoy0?t=950 for texture/uvCoord hint

				if (vertices.count(vert) == 0)
				{
					vertices[vert] = vertices.size();
					m_pVertices->push_back(vert); 
				}

				m_pIndices->push_back(vertices[vert]); 
			}
		}
	}

	std::vector<Vertex> getVertices() {
		return m_vertices; 
	}

	std::vector<uint32_t> getIndices() {
		return m_indices; 
	}
};