#pragma once
#include <vector>
#include <glm/glm.hpp>

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
		bool is_triangle = false;
		std::vector<unsigned int> n_edge;
		int fp_idx = -1; // save the newly created face point for each face temporarily. -1 means it has not been accessed
	};

	struct edge {
		unsigned int v1;     //We have to use index, instead of point itself;
		unsigned int v2;
		std::vector<unsigned int> n_face;
		bool is_border = true;

		int ep_idx = -1;    // save the newly created edge point for each line temporarily. -1 means it has not been accessed
	};
}