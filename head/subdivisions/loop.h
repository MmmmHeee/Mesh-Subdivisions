#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace loop {
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
        unsigned int v[3];
        std::vector<unsigned int> n_edge;
    };

    struct edge {
        unsigned int v1;     //We have to use index, instead of point itself;
        unsigned int v2;
        std::vector<unsigned int> n_face;
        bool is_border = true;
        unsigned int mid;    // save the new point for each line temporarily
    };
}