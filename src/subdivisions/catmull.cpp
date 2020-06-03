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
		if (curFace.size() == 3) {
			f.is_triangle = true;
		}
		// Build Vertices, we can calculate face_point along with it
		for (unsigned int j = 0; j < curFace.size(); j++) {
			unsigned int vi = curFace[j].v_idx;

			f.v[j] = vi;
			glm::vec3 pos = _obj.positions[vi];
			vertexList[vi].pos = pos;
			vertexList[vi].n_face.push_back(faceIdx);
			vertexList[vi].is_border = false;
		}
		
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
		}
		else {
			vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
			vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
		}
	}
}

void CatmullSubdivision::
subdiv()
{
	// TODO
	std::vector<vertex> oldVertexList(vertexList);
	std::vector<edge> oldEdgeList(edgeList);
	std::vector<face> oldFaceList(faceList);

	vertexList.clear(); edgeList.clear(); faceList.clear();
	//Resize the new list. The old index should be still valid to use
	vertexList.resize(oldVertexList.size());
	// Calculate and create face points
	for (int i = 0; i < oldFaceList.size(); i++) {
		face &curFace = oldFaceList[i];
		vertex facePoint;
		unsigned int fp_idx = vertexList.size();
		curFace.fp_idx = fp_idx;

		glm::vec3 fp_tmp(0); const int n = (curFace.is_triangle) ? 3 : 4;

		for (int j = 0; j < n; j++) {
			fp_tmp += oldVertexList[curFace.v[j]].pos;
		}
		facePoint.pos = fp_tmp / (float)n; //average of all points
		vertexList.push_back(std::move(facePoint));
		// The adjacent relationship cannot be determined yet.
	}
	// Calculate and create edge points
	for (int i = 0; i < oldEdgeList.size(); i++) {
		edge &curEdge = oldEdgeList[i];
		vertex edgePoint;
		unsigned int ep_idx = vertexList.size();
		curEdge.ep_idx = ep_idx;
		if (curEdge.is_border) {
			// For edges, the edge point is just its midpoint
			edgePoint.pos = 0.5f * oldVertexList[curEdge.v1].pos + 0.5f * oldVertexList[curEdge.v2].pos;
			edgePoint.is_border = true;

			// Modify adjacent relation for edgePoint
			edgePoint.b_vertex.push_back(curEdge.v1);
			edgePoint.b_vertex.push_back(curEdge.v2);
			edgePoint.n_vertex.push_back(curEdge.v1);
			edgePoint.n_vertex.push_back(curEdge.v2);
			unsigned int fp0_idx;
			fp0_idx = oldFaceList[curEdge.n_face[0]].fp_idx;
			edgePoint.n_vertex.push_back(fp0_idx);

			// And do the same to these related point
			vertexList[curEdge.v1].b_vertex.push_back(ep_idx);
			vertexList[curEdge.v2].b_vertex.push_back(ep_idx);
			vertexList[curEdge.v1].n_vertex.push_back(ep_idx);
			vertexList[curEdge.v2].n_vertex.push_back(ep_idx);
			vertexList[fp0_idx].b_vertex.push_back(ep_idx);
		}
		else {
			edgePoint.is_border = false;
			// Average between: 1. Edge midpoint / 2. Adjacent face points' segement midpoint. NOTE: the edge vertices are in old list, the face points are in new list.
			glm::vec3 fp0, fp1;
			fp0 = vertexList[oldFaceList[curEdge.n_face[0]].fp_idx].pos;
			fp1 = vertexList[oldFaceList[curEdge.n_face[1]].fp_idx].pos;
			unsigned int fp0_idx, fp1_idx;
			fp0_idx = oldFaceList[curEdge.n_face[0]].fp_idx;
			fp1_idx = oldFaceList[curEdge.n_face[1]].fp_idx;
			edgePoint.pos = 0.5f * (0.5f*(oldVertexList[curEdge.v1].pos + oldVertexList[curEdge.v2].pos) + 0.5f * (fp0 + fp1));

			edgePoint.n_vertex.push_back(curEdge.v1);
			edgePoint.n_vertex.push_back(curEdge.v2);
			edgePoint.n_vertex.push_back(fp0_idx);
			edgePoint.n_vertex.push_back(fp1_idx);

			vertexList[curEdge.v1].n_vertex.push_back(ep_idx);
			vertexList[curEdge.v2].n_vertex.push_back(ep_idx);
			vertexList[fp0_idx].n_vertex.push_back(ep_idx);
			vertexList[fp1_idx].n_vertex.push_back(ep_idx);
		}
		// n_face cannot be filled yet.
		vertexList.push_back(std::move(edgePoint));
	}
	// Up to this step, vertex adjacent relationship is filled.
	// Update old vertices. 
	for (int i = 0; i < oldVertexList.size(); i++) {
		vertex &oldVertex = oldVertexList[i];
		vertex &curVertex = vertexList[i];
		if (oldVertex.is_border) {
			curVertex.is_border = true;
			assert(oldVertex.b_vertex.size() == 2);
			curVertex.pos = (oldVertex.pos / 3.0f) + (0.5f * oldVertex.pos + 0.5f * oldVertexList[oldVertex.b_vertex[0]].pos) / 3.0f + (0.5f * oldVertex.pos + 0.5f * oldVertexList[oldVertex.b_vertex[1]].pos) / 3.0f;
		}
		else {
			curVertex.is_border = false;
			float n = oldVertex.n_face.size();
			float m1 = (n - 3.0f) / n, m2 = 1.0f / n, m3 = 2.0f / n;
			glm::vec3 avg_fp(0), avg_emp(0); // average of surrounding face points & average of edge mids
			for (int j = 0; j < oldVertex.n_face.size(); j++) {
				avg_fp += vertexList[oldFaceList[oldVertex.n_face[j]].fp_idx].pos;
			}
			avg_fp /= (float)oldVertex.n_face.size();
			for (int j = 0; j < oldVertex.n_vertex.size(); j++) {
				avg_emp += (0.5f * oldVertexList[oldVertex.n_vertex[j]].pos + 0.5f * oldVertex.pos);
			}
			avg_emp /= (float)oldVertex.n_vertex.size();

			curVertex.pos = (m1 * oldVertex.pos) + (m2 * avg_fp) + (m3 * avg_emp);
		}
	}
	// Loop on faces, to generate new faces. Triangle = 3 new Quads, Quad = 4 new Quads
	// Will need a line map this part.
	std::map<std::pair<int, int>, int> edgeIndex;
	for (int i = 0; i < oldFaceList.size(); i++) {
		face &oldFace = oldFaceList[i];
		int fp_idx = oldFace.fp_idx; // Will be used on all faces generated.
		int n = (oldFace.is_triangle) ? 3 : 4;
		for (int j = 0; j < n; j++) {
			vertex &curVertex = vertexList[oldFace.v[j]];
			int ep1_idx, ep2_idx, f_idx;
			face f; // Task: fill n_edge and v
			f_idx = faceList.size();
			// Fill v. 
			f.v[0] = oldFace.v[j];
			f.v[2] = fp_idx;
			if (j == 0) {
				ep1_idx = oldEdgeList[oldFace.n_edge[j]].ep_idx;
				ep2_idx = oldEdgeList[oldFace.n_edge[n-1]].ep_idx;
			}
			else {
				ep1_idx = oldEdgeList[oldFace.n_edge[j]].ep_idx;
				ep2_idx = oldEdgeList[oldFace.n_edge[j - 1]].ep_idx;
			}
			f.v[1] = ep1_idx;
			f.v[3] = ep2_idx;
			// Get face index into these points, too
			for (int k = 0; k < n; k++) {
				vertexList[f.v[k]].n_face.push_back(f_idx);
			}
			// Generate(or find) edge, and fill in n_edge
			for (int k = 0; k < 4; k++) {
				int edgeIdx; std::pair<int, int> lineIdx;
				if (k == 3) {
					lineIdx = getLine(f.v[k], f.v[0]);
				}
				else {
					lineIdx = getLine(f.v[k], f.v[k + 1]);
				}
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
				edgeList[edgeIdx].n_face.push_back(f_idx);
				f.n_edge.push_back(edgeIdx);
			}
			// Push f
			faceList.push_back(std::move(f));
		}
	}
}

obj_mesh CatmullSubdivision::
makeMesh()
{
	// TODO
	obj_mesh res;
	res.positions.reserve(vertexList.size());
	res.normals.reserve(vertexList.size());
	res.faces.reserve(faceList.size());
	for (auto iter = vertexList.cbegin(); iter != vertexList.end(); iter++) {
		res.positions.push_back(iter->pos);
		// not write normal
		// res.normals.push_back(iter->normal);
	}
	for (auto iter = faceList.cbegin(); iter != faceList.end(); iter++) {
		face_t newFace;
		for (unsigned int i = 0; i < 4; i++) {
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

// This will make a pair<int,int>, which represents the index of the line in the map.
std::pair<int, int> getLine(int v1, int v2)
{
	assert(v1 != v2);
	if (v1 < v2) return std::pair<int, int>(v1, v2);
	else return std::pair<int, int>(v2, v1);
}
