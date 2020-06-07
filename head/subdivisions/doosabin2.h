#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <map>
// #define FACE_NUM 12
namespace doosabin2 {
    struct vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;

        std::vector<unsigned int> n_face;
        std::vector<unsigned int> n_vertex;

        // During Subdiv
        unsigned int old_idx;
        std::vector<unsigned int> new_v_idx;
		std::map<int, int> new_n_edge_map;
    };

	struct face {        
		std::vector<unsigned int> v;
        std::vector<unsigned int> n_edge;

        // During Subdiv
        std::vector<unsigned int> new_v_idx;
		glm::vec3 face_mid_point;
    };

    struct edge {
        unsigned int v1;     //We have to use index, instead of point itself;
        unsigned int v2;
        std::vector<unsigned int> n_face;

        // During Subdiv
		unsigned int e1_near;
		unsigned int e2_near;
        glm::vec3 mid;    // save the mid point
    };
}