#pragma once
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <glm/glm.hpp>

#include "subdivision.h"

using namespace catmull;
std::pair<int, int> getLine(int v1, int v2);
void CatmullSubdivision::
loadMesh(obj_mesh &_obj) {
	edgeList.clear(); faceList.clear(); vertexList.clear();

	std::map<std::pair<int, int>, int> edgeIndex; // A pair of vertex indices is used as index of lines, and we'll make sure the smaller one is always first
	vertexList.resize(_obj.positions.size());
	// TODO
	// Loop on faces of obj file
	for (unsigned int i = 0; i < _obj.faces.size(); i++) {
		auto curFace = _obj.faces[i];
		unsigned int faceIdx = (unsigned int)faceList.size();
		face f;

		if (curFace.size() > 4) {
			std::cout << "Catmull Clark Sdv. cannot be applied on face which owns 4+ vertices!";
			edgeList.clear(); faceList.clear(); vertexList.clear();
			return;
		}
		assert(curFace.size() == 3 || curFace.size() == 4);
		// Build Vertices, we can calculate face_point along with it
		glm::vec3 fp_tmp(0);
		for (unsigned int j = 0; j < curFace.size(); j++) {
			unsigned int vi = curFace[j].v_idx;

			f.v[j] = vi;
			glm::vec3 pos = _obj.positions[vi];
			fp_tmp += pos;
			vertexList[vi].pos = pos;
			vertexList[vi].n_face.push_back(faceIdx);
			vertexList[vi].is_border = false;
		}
		f.face_point = fp_tmp / (float)curFace.size(); // Average of all vertices
		//Build Edges: edge_point cannot be calculated in this stage because we must find all adjacent faces of a edge.
		for (unsigned int j = 1; j <= curFace.size(); j++) {
			unsigned int edgeIdx; std::pair<int, int> lineIdx;
			if (j != curFace.size()) {
				lineIdx = getLine(f.v[j - 1], f.v[j]);
			}
			else
				lineIdx = getLine(f.v[j - 1], f.v[0]);
			if (edgeIndex.find(lineIdx) == edgeIndex.end()) {
				// no such edge
				edgeIdx = (int)edgeList.size();
				edge l;
				l.v1 = lineIdx.first; l.v2 = lineIdx.second;
				edgeList.push_back(std::move(l));
				edgeIndex[lineIdx] = edgeIdx;
				edgeList[edgeIdx].is_border = true;
			}
			else {
				// edge found
				edgeIdx = edgeIndex[lineIdx];
				edgeList[edgeIdx].is_border = false; // has at least two neighbour faces
			}
			edgeList[edgeIdx].n_face.push_back(faceIdx);
			f.n_edge.push_back(edgeIdx);
		}
		faceList.push_back(f);
	}
	// handle border and neighbour of vertex. At the same time we can calculate the edge point!
	for (int i = 0; i < edgeList.size(); i++) {
		edge* curEdge = &edgeList[i];
		if (curEdge->is_border) {
			vertexList[curEdge->v1].is_border = true;
			vertexList[curEdge->v2].is_border = true;
			// b_vertices are the 
			vertexList[curEdge->v1].b_vertex.push_back(curEdge->v2);
			vertexList[curEdge->v2].b_vertex.push_back(curEdge->v1);
			// For edges, the edge point is just its midpoint
			curEdge->edge_point = 0.5f * vertexList[curEdge->v1].pos + 0.5f * vertexList[curEdge->v2].pos;
		}
		else {
			vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
			vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
			// Average between: 1. Edge midpoint / 2. Adjacent face points' segement midpoint
			curEdge->edge_point = 0.5f * (0.5f*(vertexList[curEdge->v1].pos + vertexList[curEdge->v2].pos) + 0.5f*(faceList[curEdge->n_face[0]].face_point + faceList[curEdge->n_face[1]].face_point));
		}
	}
}

void CatmullSubdivision::
subdiv()
{
	// TODO
}

obj_mesh CatmullSubdivision::
makeMesh()
{
	// TODO
	return obj_mesh();
}

// This will make a pair<int,int>, which represents the index of the line in the map.
std::pair<int, int> getLine(int v1, int v2)
{
	assert(v1 != v2);
	if (v1 < v2) return std::pair<int, int>(v1, v2);
	else return std::pair<int, int>(v2, v1);
}
