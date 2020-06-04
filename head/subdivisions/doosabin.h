#pragma once
#include <vector>
#include <glm/glm.hpp>
#define FACE_NUM 12
namespace doosabin {
    struct vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;

        std::vector<unsigned int> n_face;
        std::vector<unsigned int> n_vertex;
        std::vector<unsigned int> b_vertex;
		unsigned int near_vertex[FACE_NUM];
        bool is_border = false;
    };

	struct face {
		unsigned int v[FACE_NUM];
        std::vector<unsigned int> n_edge;
        int face_num = -1;
        unsigned int new_v_idx[FACE_NUM];
		glm::vec3 face_mid_point;
		int face_side = 0;
    };

    struct edge {
        unsigned int v1;     //We have to use index, instead of point itself;
        unsigned int v2;
		unsigned int v1_near_1 = INT_MAX;
		unsigned int v1_near_2 = INT_MAX;
		unsigned int v2_near_1 = INT_MAX;
		unsigned int v2_near_2 = INT_MAX;
        std::vector<unsigned int> n_face;
        bool is_border = true;
        glm::vec3 mid;    // save the mid point
    };
}