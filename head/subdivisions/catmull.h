#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <map>

namespace catmull {
	struct vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;

		std::vector<unsigned int> n_face;
		std::vector<unsigned int> n_vertex;
		std::vector<unsigned int> b_vertex;

		bool is_border = false;
	};

	struct face {
		unsigned int v[4];
		std::vector<unsigned int> n_edge;
		glm::vec3 face_point;
	};

	struct edge {
		unsigned int v1;     //We have to use index, instead of point itself;
		unsigned int v2;
		std::vector<unsigned int> n_face;
		bool is_border = true;
		glm::vec3 edge_point;
		// unsigned int mid;    // save the new point for each line temporarily
	};
}