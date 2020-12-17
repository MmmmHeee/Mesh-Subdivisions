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

using namespace doosabin;
std::pair<int, int> lineOrder(int v1, int v2);
bool find_edge(std::map<std::pair<int, int>, int> edgeIndexx, int v1, int v2);
int times = 0;
obj_mesh temp;
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
		for (size_t j = 0; j < FACE_NUM; j++)
		{
			f.v[j] = INT_MAX;
			f.new_v_idx[j] = INT_MAX;
		}
		if (curFace.size() > FACE_NUM)
		{
			std::cout << "DooSabin error with too more edge in face" << std::endl;
			edgeList.clear(); faceList.clear(); vertexList.clear();
			return;
		}
		f.face_num = curFace.size();
		//Build Vertices
		glm::vec3 fmp_tmp(0);
		for (unsigned j = 0; j < curFace.size(); j++)
		{
			unsigned vi = curFace[j].v_idx;
			f.v[j] = vi;
			glm::vec3 pos = _obj.positions[vi];
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
				lineIdx = lineOrder(f.v[j - 1], f.v[j]);
			}
			else
			{
				lineIdx = lineOrder(f.v[j - 1], f.v[0]);
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
		vertexList[curEdge->v1].n_vertex.push_back(curEdge->v2);
		vertexList[curEdge->v2].n_vertex.push_back(curEdge->v1);
		//mid edge point
		curEdge->mid = 0.5f * vertexList[curEdge->v1].pos + 0.5f * vertexList[curEdge->v2].pos;
	}
}



void DoosabinSubdivision::
subdiv() {
    //TODO
	std::vector<vertex> oldVertexList(vertexList);
	std::vector<edge> oldEdgeList(edgeList);
	std::vector<face> oldFaceList(faceList);
	vertexList.clear(); edgeList.clear(); faceList.clear();
	//Resize the list, make sure old vertex is in position
	//vertexList.resize(oldVertexList.size());
	for (size_t i = 0; i < oldVertexList.size(); i++)
	{
		//vertexList[i] = oldVertexList[i];
		for (size_t j = 0; j < FACE_NUM; j++)
		{
			oldVertexList[i].near_vertex[j] = INT_MAX;
		}
	}
	//Calculate new point
	for (size_t i = 0; i < oldFaceList.size(); i++)
	{
		face &curFace = oldFaceList[i];
		for (size_t j = 0; j < FACE_NUM; j++)
		{
			unsigned int newv_idx = vertexList.size();
			if(curFace.v[j]<INT_MAX)
			{
				glm::vec3 mid_point = glm::vec3{-1}; glm::vec3 mid_point_1 = glm::vec3{-1};
				for (size_t k = 0; k < curFace.n_edge.size(); k++)
				{
					edge &temp_edge = oldEdgeList[curFace.n_edge[k]];
					if (temp_edge.v1 == curFace.v[j] || temp_edge.v2 == curFace.v[j])
					{
						if(mid_point == glm::vec3{-1})
						{
							mid_point = temp_edge.mid;
						}
						else
						{
							mid_point_1 = temp_edge.mid;
						}
					}
				}
				//outfile << mid_point.x << " " << mid_point.y << " " << mid_point.z << std::endl;
				//assert(mid_point!=glm::vec3{-1}&&mid_point_1!=glm::vec3{-1});
				vertex new_point;
				new_point.pos = 0.25f * (curFace.face_mid_point+mid_point+mid_point_1+oldVertexList[curFace.v[j]].pos);
				curFace.new_v_idx[j] = newv_idx;
				// load new near point to the old vertex, for linking them
				for (size_t k = 0; k < FACE_NUM; k++)
				{
					if (oldVertexList[curFace.v[j]].near_vertex[k] == INT_MAX)
					{
						oldVertexList[curFace.v[j]].near_vertex[k] = newv_idx;
						break;
					}
				}
				vertexList.push_back(std::move(new_point));
			}
		}
	}
	//edge number
	for (size_t i = 0; i < oldEdgeList.size(); i++)
	{
		edge &oldEdge = oldEdgeList[i];
		for (size_t j = 0; j < oldEdge.n_face.size(); j++)
		{
			for (size_t k = 0; k < FACE_NUM; k++)
			{
				face &oldFace = oldFaceList[oldEdge.n_face[j]];
				if (oldFace.v[k] < INT_MAX)
				{
					if (oldFace.v[k] == oldEdge.v1)
					{
						if (oldEdge.v1_near_1 == INT_MAX)
						{
							oldEdge.v1_near_1 = oldFace.new_v_idx[k];
						}
						else
						{
							oldEdge.v1_near_2 = oldFace.new_v_idx[k];
						}
					}
					if (oldFace.v[k] == oldEdge.v2)
					{
						if (oldEdge.v2_near_1 == INT_MAX)
						{
							oldEdge.v2_near_1 = oldFace.new_v_idx[k];
						}
						else
						{
							oldEdge.v2_near_2 = oldFace.new_v_idx[k];
						}
					}
				}
			}
		}
	}

	std::map<std::pair<int, int>, int> edgeIndexx;
	//link the changed face point, no more near point has been linked, face - face
	for (size_t i = 0; i < oldFaceList.size(); i++)
	{
		face newFace;
		face &oldFace = oldFaceList[i];
		glm::vec3 new_face_mid = glm::vec3{ 0 };
		for (size_t j = 0; j < FACE_NUM; j++)
		{
			newFace.v[j] = oldFace.new_v_idx[j];
			newFace.face_num = i;
			//outfile << oldFace.new_v_idx << " " << INT_MAX << std::endl;
			if (oldFace.new_v_idx[j] < INT_MAX)
			{
				new_face_mid += vertexList[oldFace.new_v_idx[j]].pos;
				if (oldFace.new_v_idx[(j + 1) % FACE_NUM] < INT_MAX)
				{
					edge new_edge;
					std::pair<int, int> lineIdx;
					unsigned int new_edge_idx = edgeList.size();
					lineIdx = std::pair<int, int>(oldFace.new_v_idx[j], oldFace.new_v_idx[(j + 1) % FACE_NUM]);
					new_edge.v1 = lineIdx.first; new_edge.v2 = lineIdx.second;
					new_edge.mid = 0.5f * (vertexList[new_edge.v1].pos + vertexList[new_edge.v2].pos);
					
					edgeIndexx[lineIdx] = new_edge_idx;
					new_edge.n_face.push_back(i);
					newFace.face_side++;
					newFace.n_edge.push_back(new_edge_idx);
					edgeList.push_back(std::move(new_edge));
					
				}
				else
				{
					edge new_edge;
					unsigned int new_edge_idx = edgeList.size();
					std::pair<int, int> lineIdx;
					lineIdx = std::pair<int, int>(oldFace.new_v_idx[j], oldFace.new_v_idx[0]);
					new_edge.v1 = lineIdx.first; new_edge.v2 = lineIdx.second;
					new_edge.mid = 0.5f * (vertexList[new_edge.v1].pos + vertexList[new_edge.v2].pos);
					edgeIndexx[lineIdx] = new_edge_idx;
					new_edge.n_face.push_back(i);
					newFace.face_side = j;
					newFace.n_edge.push_back(new_edge_idx);
					edgeList.push_back(std::move(new_edge));
					break;
				}
			}
		}
		newFace.face_mid_point = new_face_mid / (float)(newFace.face_side+1);
		faceList.push_back(std::move(newFace));
	}
	//link the edge vertex, edge- face
	for (size_t i = 0; i < oldEdgeList.size(); i++)
	{
		edge &curEdge = oldEdgeList[i];
		face new_face;
		glm::vec3 new_face_mid = glm::vec3{ 0 };
		unsigned int face_index = faceList.size();
		if (curEdge.v1_near_1 != INT_MAX && curEdge.v1_near_2 != INT_MAX && curEdge.v2_near_1 != INT_MAX && curEdge.v2_near_2 != INT_MAX)
		{
			edge new_edge, new_edge_2;
			std::pair<int, int> lineIdx;
			std::pair<int, int> lineIdx2;
			std::pair<int, int> lineIdx_tmp;
			std::pair<int, int> lineIdx2_tmp;
			unsigned int new_edge_idx = edgeList.size();
			unsigned int new_edge_idx_2 = edgeList.size()+1;
			lineIdx = std::pair<int, int>(curEdge.v1_near_1, curEdge.v1_near_2);
			lineIdx2 = std::pair<int, int>(curEdge.v2_near_1, curEdge.v2_near_2);


			//glm::vec3 vertex_pos = vertexList[lineIdx.second].pos; glm::vec3 vertex_pos2 = vertexList[lineIdx.first].pos;
			//glm::vec3 vertex_pos3 = vertexList[lineIdx2.second].pos; glm::vec3 vertex_pos4 = vertexList[lineIdx2.first].pos;
			
			glm::vec3 edge_1 = vertexList[lineIdx.second].pos - vertexList[lineIdx.first].pos;
			glm::vec3 edge_2 = vertexList[lineIdx2.first].pos - vertexList[lineIdx.first].pos;
			float angle = glm::angle(edge_1, edge_2);
			if (find_edge(edgeIndexx, lineIdx.first, lineIdx2.first))
			{
				new_face.v[0] = lineIdx.first; new_face.v[1] = lineIdx.second;
				new_face.v[3] = lineIdx2.first; new_face.v[2] = lineIdx2.second;
				lineIdx_tmp = std::pair<int, int>(lineIdx.first, lineIdx.second);
				lineIdx2_tmp = std::pair<int, int>(lineIdx2.second, lineIdx2.first);
			}
			else
			{
				new_face.v[0] = lineIdx.first; new_face.v[3] = lineIdx.second;
				new_face.v[1] = lineIdx2.first; new_face.v[2] = lineIdx2.second;
				lineIdx_tmp = std::pair<int, int>(lineIdx.second, lineIdx.first);
				lineIdx2_tmp = std::pair<int, int>(lineIdx2.first, lineIdx2.second);
			}
			new_edge.v1 = lineIdx.first; new_edge.v2 = lineIdx.second;
			new_edge_2.v1 = lineIdx2.first; new_edge_2.v2 = lineIdx2.second;

			new_edge.mid = 0.5f * (vertexList[new_edge.v1].pos + vertexList[new_edge.v2].pos);
			new_face_mid += 0.25f * (vertexList[new_edge.v1].pos + vertexList[new_edge.v2].pos);
			new_edge_2.mid = 0.5f * (vertexList[new_edge_2.v1].pos + vertexList[new_edge_2.v2].pos);
			new_face_mid += 0.25f * (vertexList[new_edge_2.v1].pos + vertexList[new_edge_2.v2].pos);

			edgeIndexx[lineIdx_tmp] = new_edge_idx;
			edgeIndexx[lineIdx2_tmp] = new_edge_idx_2;

			new_face.n_edge.push_back(new_edge_idx);
			new_face.n_edge.push_back(new_edge_idx_2);

			new_edge.n_face.push_back(face_index);
			edgeList.push_back(std::move(new_edge));
			new_edge_2.n_face.push_back(face_index);
			edgeList.push_back(std::move(new_edge_2));
		}
		new_face.face_side = 3; new_face.face_num = face_index;
		new_face.face_mid_point = new_face_mid;
		faceList.push_back(std::move(new_face));
	}

	


	// point - face
	for (size_t i = 0; i < oldVertexList.size(); i++)
	{
		face new_face;
		int face_insert = 0;
		glm::vec3 new_face_mid = glm::vec3{ 0 };
		vertex &curVertex = oldVertexList[i];
		int tmp_save[2 * FACE_NUM];
		int pass_tmp[2 * FACE_NUM];
		for (size_t j = 0; j < 2*FACE_NUM; j++)
		{
			tmp_save[j] = -1;
			pass_tmp[j] = -1;
		}
		for (size_t j = 0; j < FACE_NUM; j++)
		{
			if (curVertex.near_vertex[j] < INT_MAX)
			{
				for (size_t k = 0; k < FACE_NUM; k++)
				{
					if (curVertex.near_vertex[k] < INT_MAX)
					{
						if (find_edge(edgeIndexx, curVertex.near_vertex[j], curVertex.near_vertex[k]))
						{
							tmp_save[face_insert++] = curVertex.near_vertex[j];
							tmp_save[face_insert++] = curVertex.near_vertex[k];
							//new_face.v[face_insert++] = curVertex.near_vertex[k];
						}
					}
				}
			}
		}
		
		for (size_t j = 0; j < 2*FACE_NUM; j+=2)
		{
			if (tmp_save[j] != -1)
			{
				if (find_edge(edgeIndexx, tmp_save[j], tmp_save[j+1]))
				{
					pass_tmp[0] = tmp_save[j+1];
					pass_tmp[1] = tmp_save[j];
					break;
				}
			}
		}
		int pass_pointer = 1;
		for (size_t j = 0; j < 2*FACE_NUM; j+=2)
		{
			if (tmp_save[j] != -1)
			{
				if (tmp_save[j] == pass_tmp[pass_pointer])
				{
					if (tmp_save[j + 1] != pass_tmp[0])
					{
						pass_pointer++;
						pass_tmp[pass_pointer] = tmp_save[j + 1];
					}
				}
				else if (tmp_save[j + 1] == pass_tmp[pass_pointer])
				{
					if (tmp_save[j] != pass_tmp[0])
					{
						pass_pointer++;
						pass_tmp[pass_pointer] = tmp_save[j];
					}
				}
			}
			else
			{
				break;
			}
		}
		for (size_t j = 0; j <= pass_pointer; j++)
		{
			new_face.v[j] = pass_tmp[j];
			new_face_mid += vertexList[pass_tmp[j]].pos;
		}
		
		new_face.face_num = faceList.size();
		new_face.face_side = pass_pointer;
		new_face.face_mid_point = new_face_mid / (float)(face_insert);
		faceList.push_back(std::move(new_face));
	}
	for (size_t i = 0; i < faceList.size(); i++)
	{
		face &curFace = faceList[i];
		int face_num = curFace.face_num;
		for (size_t j = 0; j < curFace.n_edge.size(); j++)
		{
			edge & curEdge = edgeList[curFace.n_edge[j]];
			curEdge.n_face.push_back(face_num);
		}
	}
}

obj_mesh DoosabinSubdivision::
makeMesh()
{
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
		for (unsigned int i = 0; i <= iter->face_side; i++) {
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

obj_mesh DoosabinSubdivision::
doosabin_makeMesh()
{
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
		for (unsigned int i = 0; i <= iter->face_side; i++) {
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

std::pair<int, int> lineOrder(int v1, int v2)
{
	//assert(v1 != v2);
	if (v1 < v2) return std::pair<int, int>(v1, v2);
	else return std::pair<int, int>(v2, v1);
}


bool find_edge(std::map<std::pair<int, int>, int> edgeIndexx, int v1, int v2)
{
	std::pair<int, int> lineidx = std::pair<int, int>(v1, v2);
	if (edgeIndexx.find(lineidx) == edgeIndexx.end())
	{
		return false;
	}
	return true;
}
		

