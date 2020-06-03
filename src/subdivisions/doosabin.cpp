#pragma once
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <glm/glm.hpp>

#include "subdivision.h"

// Decide whether a edge is boundary: Find adj faces of one of the two vertexs, and check whether any of them used the other vertex.
// Use face as the iterator of loop to find edges

using namespace doosabin;

void DoosabinSubdivision::
loadMesh(obj_mesh & _obj) {
    edgeList.clear(); faceList.clear(); vertexList.clear();

    std::map<std::pair<int, int>, int> edgeIndex; // avoid dup edges
    vertexList.resize(_obj.positions.size());
	//TODO
	//Loop for faces
	for (unsigned i = 0; i < _obj.faces.size(); i++)
	{
		auto curFace = _obj.faces[i];
		unsigned faceIdx = (unsigned)faceList.size();
		face f;
		if (curFace.size() > FACE_NUM)
		{
			std::cout << "DooSabin error with too more edge in face" << std::endl;
			edgeList.clear(); faceList.clear(); vertexList.clear();
			return;
		}
		//Build Vertices
		glm::vec3 fmp_tmp(0);
		for (unsigned j = 0; j < curFace.size(); j++)
		{
			unsigned vi = curFace[j].v_idx;
			f.v[j] = vi;
			glm::vec3 pos = _obj.position[vi];
			fmp_tmp += pos;
			vertexList[vi].pos = pos;
			vertexList[vi].n_face.push_back(faceIdx);
			vertexList[vi].is_border = false;
		}
		//calculate face mid point;
		f.face_mid_point = fmp_tmp / (float)curFace.size();
		//Build Edge
		for (unsigned j = 1; j <= curFace.size(); j++)
		{
			unsigned edgeIdx; std::pair<int, int> lineIdx;
			if (j != curFace.size()) 
			{
				lineIdx = getLine(f.v[j - 1], f.v[j]);
			}
			else
			{
				lineIdx = getLine(f.v[j - 1], f.v[0]);
			}
			if (edgeIndex.find(lineIdx) == edgeIndex.end())
			{
				//no edge
				edgeIdx = (int)edgeList.size();
				edge l;
				l.v1 = lineIdx.first; l.v2 = lineIdx.second;
				edgeList.push_back(std::move(l));
				edgeIndex[lineIdx] = edgeIdx;
				edgeList[edgeIdx].is_border = true;
			}
			else
			{
				//edge found
				edgeIdx = edgeIndex[lineIdx];
				edgeList[edgeIdx].is_border = false;
			}
			edgeList[edgeIdx].n_face.push_back(faceIdx);
			f.n_edge.push_back(edgeIdx);
		}
		faceList.push_back(f);
	}
	for (int i = 0; i < edgeList.size(); i++)
	{
		edge* curEdge = &edgeList[i];
		if (curEdge->is_border)
		{
			vertexList[curEdge->v1].is_border = true;
			vertexList[curEdge->v2].is_border = true;
			vertexList[curEdge->v1].b_vertex.push_back(curEdge->v2);
			vertexList[curEdge->v2].b_vertex.push_back(curEdge->v1);
		}
		else
		{
			vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
			vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
		}
		//mid edge point
		curEdge->mid = 0.5f * vertexList[curEdge->v1].pos + 0.5f * vertexList[curEdge->v2].pos;
	}

}

void DoosabinSubdivision::
subdiv() {
    //TODO
}

obj_mesh DoosabinSubdivision::
makeMesh() 
{
    //TODO
	return obj_mesh();
}

std::pair<int, int> getLine(int v1, int v2)
{
	assert(v1 != v2);
	if (v1 < v2) return std::pair<int, int>(v1, v2);
	else return std::pair<int, int>(v2, v1);
}
		

