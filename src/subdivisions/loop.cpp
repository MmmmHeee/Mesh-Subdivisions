#pragma once
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <glm/glm.hpp>

#include "subdivision.h"

// Decide whether a edge is boundary: Find adj faces of one of the two vertexs, and check whether any of them used the other vertex.
// Use face as the iterator of loop to find edges

using namespace loop;

void LoopSubdivision::
loadMesh(obj_mesh & _obj) {
    edgeList.clear(); faceList.clear(); vertexList.clear();

    std::map<std::pair<int, int>, int> edgeIndex; // avoid dup edges
    vertexList.resize(_obj.positions.size());

    // traverse faces
    // we only take vertex position into consideration
    // different normals of one vertex will be degenerated into one by doing average
    for (unsigned int i = 0; i < _obj.faces.size(); i += 1) {
        auto curFace = _obj.faces[i];
        unsigned int faceIdx = (unsigned int) faceList.size();
        face f;
        
        if (curFace.size() != 3) {
            std::cout << "failed to apply loop subdivision on non-triangle mesh" << std::endl;
            edgeList.clear(); faceList.clear(); vertexList.clear();
            return;
        }

        // build 3 vertex
        for (unsigned int j = 0; j <= 2; j += 1) {
            unsigned int vi = curFace[j].v_idx;
            int vn = curFace[j].vn_idx;

            f.v[j] = vi;
            glm::vec3 pos = _obj.positions[vi];
            
            glm::vec3 normal;
            if (vn != -1) {
                normal = _obj.normals[vn]; // take every normal and add them together
            } else {
                normal = glm::vec3(NAN, NAN, NAN); // no normal
            }

            vertexList[vi].pos = pos; 
            vertexList[vi].normal = vertexList[vi].normal + normal; // the normal will be normalized finally
            vertexList[vi].n_face.push_back(faceIdx);
            vertexList[vi].is_border = false;
        }

        // build edges:
        // f.v[0]-f.v[1] -> f.n_edge[0] 
        // f.v[1]-f.v[2] -> f.n_edge[1]
        // f.v[2]-f.v[0] -> f.n_edge[2]
        for (unsigned int j = 0; j <= 2; j += 1) {
            unsigned int edgeIdx, mini, maxi;
            if (j != 2) {
                mini = std::min(f.v[j], f.v[j + 1]);
                maxi = std::max(f.v[j], f.v[j + 1]);
            } else {
                mini = std::min(f.v[2], f.v[0]);
                maxi = std::max(f.v[2], f.v[0]);
            }
            auto key = std::pair<int, int>(mini, maxi); // prevent duplicate edges

            if (edgeIndex.find(key) == edgeIndex.end()) {
                // no such edge
                edgeIdx = (int) edgeList.size();
                edge l;
                l.v1 = key.first; l.v2 = key.second;
                edgeList.push_back(std::move(l));
                edgeIndex[key] = edgeIdx;
                edgeList[edgeIdx].is_border = true;
            } else {
                // edge found
                edgeIdx = edgeIndex[key];
                edgeList[edgeIdx].is_border = false; // has at least two neighbour faces
            }
            edgeList[edgeIdx].n_face.push_back(faceIdx);
            f.n_edge.push_back(edgeIdx);
        }

        faceList.push_back(f);
    }

    // handle border and neighbour of vertex
    for (auto curEdge = edgeList.cbegin(); curEdge != edgeList.end(); curEdge++) {
        if (curEdge->is_border) {
            vertexList[curEdge->v1].is_border = true;
            vertexList[curEdge->v2].is_border = true;
            
            vertexList[curEdge->v1].b_vertex.push_back(curEdge->v2);
            vertexList[curEdge->v2].b_vertex.push_back(curEdge->v1);
        } else {
            vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
            vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
        }
    }

    // handle vertex normal by doing average
    for (auto curVertex = vertexList.begin(); curVertex != vertexList.end(); curVertex++) {
        curVertex->normal = curVertex->normal * (1.0f / curVertex->n_face.size());
    }
}

void LoopSubdivision::
subdiv() {
    std::vector<vertex> oldVertexList(vertexList);
    std::vector<edge> oldEdgeList(edgeList);
    std::vector<face> oldFaceList(faceList);
    
    vertexList.clear(); edgeList.clear(); faceList.clear();
    vertexList.resize(oldVertexList.size());

    // generate new vertices for each edge
    // new vertices are saved in vertexList after original vertices
    for (auto curEdge = oldEdgeList.begin(); curEdge != oldEdgeList.end(); curEdge++) {        
        vertex v0 = oldVertexList[curEdge->v1];
        vertex v1 = oldVertexList[curEdge->v2];
        vertex mid;
        int midIdx = (int) vertexList.size();
        if (curEdge->is_border) { // generate vertex on border edge
            mid.pos = (0.5f * v0.pos + 0.5f * v1.pos);
            mid.normal = glm::normalize(0.5f * v0.normal + 0.5f * v1.normal);
            mid.is_border = true;
        } else { // not border
            // find the target vertex which is on two neightbor faces but not current edge
            face n_face[] = {oldFaceList[curEdge->n_face[0]], oldFaceList[curEdge->n_face[1]]};
            vertex v_far[2];
            for (unsigned int j = 0; j < 2; j += 1) {
                // for each face and find one vertex
                for (unsigned int k = 0; k < 3; k += 1) {
                    // on two neightbor faces but not current edge
                    if (n_face[j].v[k] != curEdge->v1 && n_face[j].v[k] != curEdge->v2) {
                        v_far[j] = oldVertexList[n_face[j].v[k]];
                        break;
                    }
                }
            }
            mid.pos = 0.375f * v0.pos + 0.375f * v1.pos + \
                      0.125f * v_far[0].pos + 0.125f * v_far[1].pos;
            mid.normal = glm::normalize(0.375f * v0.normal + 0.375f * v1.normal + \
                      0.125f * v_far[0].normal + 0.125f * v_far[1].normal);
            mid.is_border = false;
        }
        vertexList.push_back(std::move(mid));
        curEdge->mid = midIdx; // save in the edge temporarily
    }
   
    // move original vertices
    for (unsigned int vi = 0; vi < oldVertexList.size(); vi += 1) {
        vertex cur = oldVertexList[vi];
        if (cur.is_border) {
            vertex new_v[] = {oldVertexList[cur.b_vertex[0]], oldVertexList[cur.b_vertex[1]]}; // neighbour vertices
            vertexList[vi].pos = 0.75f * cur.pos + 0.125f * new_v[0].pos + 0.125f * new_v[1].pos;
            vertexList[vi].normal = glm::normalize(0.75f * cur.normal + 0.125f * new_v[0].normal + 0.125f * new_v[1].normal);
        } else {
            unsigned int k = (unsigned int) cur.n_vertex.size();
            float beta = 1.0f/k * (0.625f - pow(0.375f + 0.25f * cosf(2 * 3.14159265359f / k), 2));

            glm::vec3 pos = (1.0f - k * beta) * cur.pos;
            glm::vec3 normal = (1.0f - k * beta) * cur.normal;
            for (unsigned int j = 0; j < k; j += 1) { // use information fom neighbor vertices
                pos = pos + beta * oldVertexList[cur.n_vertex[j]].pos;
                normal = normal + beta * oldVertexList[cur.n_vertex[j]].normal;
            }
            vertexList[vi].pos = pos;
            vertexList[vi].normal = glm::normalize(normal);
        }
    }

    edgeList.resize(oldEdgeList.size() * 2);
    faceList.resize(oldFaceList.size() * 4);

    // build new faces and new inner_edges
    for (unsigned int fi = 0; fi < oldFaceList.size(); fi += 1) {
        face cur = oldFaceList[fi];
        edge old_edges[] = {oldEdgeList[cur.n_edge[0]], oldEdgeList[cur.n_edge[1]], oldEdgeList[cur.n_edge[2]]};

        unsigned int inner_newedges_index[3];
        edge inner_newedges[3]; // inner_newedges[0] is closest to cur.v[0]
        unsigned int outer_newedges_index[3][2]; // outer_newedges[i][j] saved to index 2 * cur.n_edges[i] + j
        edge outer_newedges[3][2]; // outer_newedges[i] are two edges splitted from old_edges[i]

        unsigned int outer_newfaces_index[3] = {4 * fi, 4 * fi + 1, 4 * fi + 2};
        face outer_newfaces[3];// cur.v[k] in new_faces[k], put in F_list_new[4 * fi + k]

        unsigned int inner_newfaces_index = 4 * fi + 3;
        face inner_newfaces; // inner face, put in F_list_new[4 * fi + 3]

        // generate inner_newedges
        for (unsigned int i = 0; i <= 2; i += 1) {
            // 0 -> old_edges[0]mid, [2].mid
            // 1 -> 0, 1
            // 2 -> 2, 1
            inner_newedges[i].v1 = old_edges[i].mid;
            if (i != 0) { 
                inner_newedges[i].v2 = old_edges[i-1].mid;
            } else { 
                inner_newedges[0].v2 = old_edges[2].mid;
            }
            inner_newedges[i].n_face.push_back(4 * fi + i);
            inner_newedges[i].n_face.push_back(4 * fi + 3);
            inner_newedges[i].is_border = false;
            inner_newedges_index[i] = (int) edgeList.size();
            edgeList.push_back(inner_newedges[i]);
        }

        // generate outer_newedges
        for (unsigned int i = 0; i <= 2; i += 1) {
            outer_newedges[i][0].v1 = cur.v[i];
            outer_newedges[i][0].v2 = old_edges[i].mid;
            outer_newedges[i][1].v1 = old_edges[i].mid;
            if (i != 2) {
                outer_newedges[i][1].v2 = cur.v[i + 1];
            } else {
                outer_newedges[2][1].v2 = cur.v[0];
            }
            
            // Sharing edge will be the same index for both faces
            // avoid duplicate edges by giving same index
            if (outer_newedges[i][0].v1 < outer_newedges[i][1].v2) {
                outer_newedges_index[i][0] = 2 * cur.n_edge[i];
                outer_newedges_index[i][1] = 2 * cur.n_edge[i] + 1;
            } else {
                outer_newedges_index[i][0] = 2 * cur.n_edge[i] + 1;
                outer_newedges_index[i][1] = 2 * cur.n_edge[i];
            }

            for (unsigned int j = 0; j <= 1; j += 1) {
                edge *l = &edgeList[outer_newedges_index[i][j]];
                l->v1 = outer_newedges[i][j].v1;
                l->v2 = outer_newedges[i][j].v2;
                l->is_border = old_edges[i].is_border;
                if (i + j != 3) {
                    l->n_face.push_back(4 * fi + i + j);
                } else {
                    l->n_face.push_back(4 * fi);
                }
            }
        }

        // generate outer_newfaces
        for (unsigned int i = 0; i <= 2; i += 1) {
            face *f = &faceList[outer_newfaces_index[i]];
            f->v[0] = cur.v[i];
            f->v[1] = old_edges[i].mid;
            f->n_edge.push_back(outer_newedges_index[i][0]);
            f->n_edge.push_back(inner_newedges_index[i]);
            if (i != 0) {
                f->v[2] = old_edges[i-1].mid;
                f->n_edge.push_back(outer_newedges_index[i-1][1]);
            } else {
                f->v[2] = old_edges[2].mid;
                f->n_edge.push_back(outer_newedges_index[2][1]);
            }
        }

        // generate inner_newfaces
        face *inner_face = &faceList[inner_newfaces_index];
        for (unsigned int i = 0; i <= 2; i += 1) {
            inner_face->v[i] = old_edges[i].mid;
            if (i != 2) {
                inner_face->n_edge.push_back(inner_newedges_index[i + 1]);
            } else {
                inner_face->n_edge.push_back(inner_newedges_index[0]);
            }
        }
    }
    
    // handle border and neighbour of vertex
    for (auto curEdge = edgeList.cbegin(); curEdge != edgeList.end(); curEdge++) {
        if (curEdge->is_border) {
            vertexList[curEdge->v1].is_border = true;
            vertexList[curEdge->v2].is_border = true;
            
            vertexList[curEdge->v1].b_vertex.push_back(curEdge->v2);
            vertexList[curEdge->v2].b_vertex.push_back(curEdge->v1);
        } else {
            vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
            vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
        }
    }
}

obj_mesh LoopSubdivision::
makeMesh() 
{
    obj_mesh res;
    
    res.positions.reserve(vertexList.size());
    res.normals.reserve(vertexList.size());
    for (auto iter = vertexList.cbegin(); iter != vertexList.end(); iter++) {
        res.positions.push_back(iter->pos);
        // not write normal
        // res.normals.push_back(iter->normal);
    }
    res.faces.reserve(faceList.size());
    for (auto iter = faceList.cbegin(); iter != faceList.end(); iter++) {
        face_t newFace;
        for (unsigned int i = 0; i < 3; i += 1) {
            vertex_index vi;
            vi.v_idx = iter->v[i];
            vi.vn_idx = -1; // not write normal
            vi.vt_idx = -1;
            newFace.push_back(vi);
        }
        res.faces.push_back(newFace);
    }
	return res;
}
		

