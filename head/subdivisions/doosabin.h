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

        bool is_border = false;
    };

    struct face {
        unsigned int v[FACE_NUM];
        std::vector<unsigned int> n_edge;
		glm::vec3 face_mid_point;
    };

    struct edge {
        unsigned int v1;     //We have to use index, instead of point itself;
        unsigned int v2;
        std::vector<unsigned int> n_face;
        bool is_border = true;
        glm::vec3 mid;    // save the mid point
    };
}