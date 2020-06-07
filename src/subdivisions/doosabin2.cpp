#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "subdivision.h"

// Decide whether a edge is boundary: Find adj faces of one of the two vertexs, and check whether any of them used the other vertex.
// Use face as the iterator of loop to find edges

using namespace doosabin2;

static std::pair<int, int> lineOrder(int v1, int v2);
static bool find_edge(std::map<std::pair<int, int>, int> edgeIndexMap, int v1, int v2);
static bool find_edge(std::map<std::pair<int, int>, int> edgeIndexMap, std::pair<int, int>);

void Doosabin2Subdivision::
loadMesh(obj_mesh & _obj) {
    edgeList.clear(); faceList.clear(); vertexList.clear();

    std::map<std::pair<int, int>, int> edgeIndexMap; // avoid dup edges
    vertexList.resize(_obj.positions.size());
	//TODO
	//Loop for faces
	for (unsigned i = 0; i < _obj.faces.size(); i++)
	{
		auto curFace = _obj.faces[i];
		unsigned int faceIdx = (unsigned int) faceList.size();
		face f;

		f.v.resize(curFace.size());
		//Build Vertices
		for (unsigned j = 0; j < curFace.size(); j++)
		{
			unsigned vi = curFace[j].v_idx;
			f.v[j] = vi;
			vertexList[vi].pos = _obj.positions[vi];
			vertexList[vi].n_face.push_back(faceIdx);
		}
		
		// put face mid-point calculation into subdiv()
		
		//Build Edge
		for (unsigned j = 1; j <= curFace.size(); j++)
		{
			unsigned edgeIdx; 
			std::pair<int, int> lineIdx;
			unsigned int v1; unsigned int v2;
			if (j != curFace.size()) {
				v1 = f.v[j - 1];
				v2 = f.v[j];
			} else {
				v1 = f.v[j - 1];
				v2 = f.v[0];
			}
			lineIdx = lineOrder(v1, v2);
			if (!find_edge(edgeIndexMap, lineIdx))
			{
				//no edge
				edgeIdx = (int) edgeList.size();
				edge l;
				l.v1 = lineIdx.first; l.v2 = lineIdx.second;
				edgeList.push_back(std::move(l));
				edgeIndexMap[lineIdx] = edgeIdx;
			} else {
				//edge found
				edgeIdx = edgeIndexMap[lineIdx];
			}

			edgeList[edgeIdx].n_face.push_back(faceIdx);
			f.n_edge.push_back(edgeIdx);
		}
		faceList.push_back(f);
	}
	
	for (int i = 0; i < edgeList.size(); i++)
	{
		edge* curEdge = &edgeList[i];
		vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
		vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);

		// put mid edge calculation into subdiv()
	}
}



void Doosabin2Subdivision::
subdiv() {
	std::vector<vertex> oldVertexList(vertexList);
	std::vector<edge> oldEdgeList(edgeList);
	std::vector<face> oldFaceList(faceList);
	vertexList.clear(); edgeList.clear(); faceList.clear();
	
	// Calculate mid point for all edges
	for (size_t i = 0; i < oldEdgeList.size(); i++)
	{
		edge* curEdge = &oldEdgeList[i];
		curEdge->mid = 0.5f * oldVertexList[curEdge->v1].pos + 0.5f * oldVertexList[curEdge->v2].pos;
	}

	// Calculate mid point for all faces
	for (size_t i = 0; i < oldFaceList.size(); i++) {
		face* curFace = &oldFaceList[i];
		glm::vec3 mid = glm::vec3(0);
		for (size_t j = 0; j < curFace->v.size(); j++) {
			mid += oldVertexList[curFace->v[j]].pos;
		}
		mid = mid / (float) curFace->v.size();
		curFace->face_mid_point = mid;
	}
	
	// Calculate new point for all face
	for (size_t i = 0; i < oldFaceList.size(); i++)
	{
		face &curFace = oldFaceList[i];

		// For all vertex
		for (size_t j = 0; j < curFace.v.size(); j++)
		{
			unsigned int curv_idx = curFace.v[j];
			vertex &curVertex = oldVertexList[curv_idx];
			unsigned int newv_idx = vertexList.size();

			glm::vec3 mid_point_sum = glm::vec3(0);

			// iterate over all edges of curFace to find two neighbour edge of curVertex
			for (size_t k = 0; k < curFace.n_edge.size(); k++)
			{	
				edge &curEdge = oldEdgeList[curFace.n_edge[k]];
				if (curEdge.v1 == curv_idx || curEdge.v2 == curv_idx)
				{
					mid_point_sum += curEdge.mid;
				}
			}

			vertex new_point;
			new_point.pos = 0.25f * (curFace.face_mid_point + mid_point_sum + oldVertexList[curv_idx].pos);
			new_point.old_idx = curv_idx;
			curVertex.new_v_idx.push_back(newv_idx);
			curFace.new_v_idx.push_back(newv_idx);
			vertexList.push_back(std::move(new_point));
		}
	}

	std::map<std::pair<int, int>, int> FE2newEdgeMap; // map from old face index with old edge index to new edge index
	std::map<std::pair<int, int>, int> edgeIndexMap; // prevent dup line
	
	//link the changed face point, no more near point has been linked, face - face
	for (size_t i = 0; i < oldFaceList.size(); i++)
	{
		face newFace;
		face &oldFace = oldFaceList[i];
		
		newFace.v = std::vector<unsigned int>(oldFace.new_v_idx);
		
		for (size_t j = 1; j <= oldFace.new_v_idx.size(); j++) {
			edge new_edge;
			std::pair<int, int> lineIdx;
			unsigned int new_edge_idx = edgeList.size();
			std::pair<int, int> oldEdgeIdx;

			unsigned int v1; unsigned int v2;
			if (j != oldFace.new_v_idx.size()) {
				v1 = oldFace.new_v_idx[j - 1];
				v2 = oldFace.new_v_idx[j];
			} else {
				v1 = oldFace.new_v_idx[j - 1];
				v2 = oldFace.new_v_idx[0];
			}
			lineIdx = lineOrder(v1, v2);
			new_edge.v1 = v1; new_edge.v2 = v2;

			vertexList[new_edge.v1].n_vertex.push_back(new_edge.v2);
			vertexList[new_edge.v2].n_vertex.push_back(new_edge.v1);
			
			vertexList[new_edge.v1].n_face.push_back(i);
			vertexList[new_edge.v2].n_face.push_back(i);
			
			new_edge.n_face.push_back(i);
			newFace.n_edge.push_back(new_edge_idx);

			edgeList.push_back(std::move(new_edge));
			edgeIndexMap[lineIdx] = new_edge_idx;
			FE2newEdgeMap[std::pair<int, int>(i, oldFace.n_edge[j - 1])] = new_edge_idx;
		}
		faceList.push_back(std::move(newFace));
	}

	//link the edge vertex, edge- face
	for (size_t i = 0; i < oldEdgeList.size(); i++)
	{
		edge &curEdge = oldEdgeList[i];

		if (curEdge.n_face.size() == 2) {
			face new_face;
			unsigned int face_index = faceList.size();

			edge new_edge_1;
			edge new_edge_2;
			unsigned int new_edge_idx1 = edgeList.size();
			unsigned int new_edge_idx2 = edgeList.size() + 1;

			edge added_edge_1 = edgeList[FE2newEdgeMap[std::pair<int, int>(curEdge.n_face[0], i)]];
			edge added_edge_2 = edgeList[FE2newEdgeMap[std::pair<int, int>(curEdge.n_face[1], i)]];
			
			// index for new edges
			new_edge_1.v1 = added_edge_1.v2; new_edge_1.v2 = added_edge_2.v1;
			new_edge_2.v1 = added_edge_2.v2; new_edge_2.v2 = added_edge_1.v1;

			new_face.v.push_back(added_edge_2.v2); 
			new_face.v.push_back(added_edge_2.v1); 
			new_face.v.push_back(added_edge_1.v2); 
			new_face.v.push_back(added_edge_1.v1); 

			// handle vertex neighbor info 
			vertexList[new_edge_1.v1].n_vertex.push_back(new_edge_1.v2);
			vertexList[new_edge_1.v2].n_vertex.push_back(new_edge_1.v1);

			vertexList[new_edge_2.v1].n_vertex.push_back(new_edge_2.v2);
			vertexList[new_edge_2.v2].n_vertex.push_back(new_edge_2.v1);
			
			vertexList[new_edge_1.v1].n_face.push_back(face_index);
			vertexList[new_edge_1.v2].n_face.push_back(face_index);
			vertexList[new_edge_2.v1].n_face.push_back(face_index);
			vertexList[new_edge_2.v2].n_face.push_back(face_index);

			// handle face and edge neighbor info
			new_edge_1.n_face.push_back(face_index);
			new_edge_2.n_face.push_back(face_index);

			// add new edge
			edgeList.push_back(new_edge_1);
			edgeList.push_back(new_edge_2);
			std::pair<int, int> newEdgeIdx1 = lineOrder(new_edge_1.v1, new_edge_1.v2);
			std::pair<int, int> newEdgeIdx2 = lineOrder(new_edge_2.v1, new_edge_2.v2);
			edgeIndexMap[newEdgeIdx1] = new_edge_idx1;
			edgeIndexMap[newEdgeIdx2] = new_edge_idx2;

			// add edges for old vertex per face
			oldVertexList[vertexList[new_edge_1.v1].old_idx].new_n_edge_map[new_edge_1.v1] = new_edge_1.v2;
			oldVertexList[vertexList[new_edge_2.v1].old_idx].new_n_edge_map[new_edge_2.v1] = new_edge_2.v2;

			// should have been added by face iteration
			std::pair<int, int> addedEdgeIdx1 = lineOrder(added_edge_1.v1, added_edge_1.v2);
			std::pair<int, int> addedEdgeIdx2 = lineOrder(added_edge_2.v1, added_edge_2.v2);
			unsigned int added_edge_idx1 = edgeIndexMap[addedEdgeIdx1];
			unsigned int added_edge_idx2 = edgeIndexMap[addedEdgeIdx2];

			// handle neighbor info
			edgeList[added_edge_idx1].n_face.push_back(face_index);
			edgeList[added_edge_idx2].n_face.push_back(face_index);

			new_face.n_edge.push_back(added_edge_idx2);
			new_face.n_edge.push_back(new_edge_idx1);
			new_face.n_edge.push_back(added_edge_idx1);
			new_face.n_edge.push_back(new_edge_idx2);

			faceList.push_back(std::move(new_face));
		}
	}


	// point - face
	for (size_t i = 0; i < oldVertexList.size(); i++)
	{
		vertex &curVertex = oldVertexList[i];
		if (curVertex.n_face.size() >= 3) {
			face new_face;
			size_t face_index = faceList.size();
			
			unsigned int start_idx = curVertex.new_v_idx[0];
			unsigned int cur_idx = start_idx;
			do {
				new_face.v.push_back(cur_idx);
				unsigned int next_idx = curVertex.new_n_edge_map[cur_idx];
				unsigned int edgeIdx = edgeIndexMap[lineOrder(cur_idx, next_idx)];
				edgeList[edgeIdx].n_face.push_back(face_index);
				new_face.n_edge.push_back(edgeIdx);
				cur_idx = next_idx;
			} while(cur_idx != start_idx);

			faceList.push_back(std::move(new_face));
		}
	}
}

obj_mesh Doosabin2Subdivision::
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
        for (unsigned int i = 0; i < iter->v.size(); i += 1) {
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


static std::pair<int, int> lineOrder(int v1, int v2)
{
	//assert(v1 != v2);
	if (v1 < v2) return std::pair<int, int>(v1, v2);
	else return std::pair<int, int>(v2, v1);
}

static bool find_edge(std::map<std::pair<int, int>, int> edgeIndexMap, int v1, int v2)
{
	std::pair<int, int> lineIdx = lineOrder(v1, v2);
	if (edgeIndexMap.find(lineIdx) == edgeIndexMap.end())
		return false;

	return true;
}
		
static bool find_edge(std::map<std::pair<int, int>, int> edgeIndexMap, std::pair<int, int> lineIdx)
{
	if (edgeIndexMap.find(lineIdx) == edgeIndexMap.end())
		return false;

	return true;
}


