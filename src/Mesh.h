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
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

public:
	Mesh() {

	}

	void create(const char* path) {
		tinyobj::attrib_t vertexAttributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errorString;
		std::string warnString;

		bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warnString, &errorString, path);

		if (!success)
		{
			throw std::runtime_error(errorString); 
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

				/*vert.uvCoord= {
					vertexAttributes.texcoords[2 * index.texcoord_index + 0],
					vertexAttributes.texcoords[2 * index.texcoord_index + 1]
				};*/

				vert.uvCoord = {
					vertexAttributes.texcoords[2 * index.texcoord_index + 0],
					1.0f - vertexAttributes.texcoords[2 * index.texcoord_index + 1]
				};

				vert.color = { 1.0f, 1.0f, 1.0f };

				if (vertices.count(vert) == 0)
				{
					vertices[vert] = vertices.size();
					m_vertices.push_back(vert); 
				}

				m_indices.push_back(vertices[vert]); 
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