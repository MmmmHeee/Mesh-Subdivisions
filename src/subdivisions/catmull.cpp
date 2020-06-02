#pragma once
#include <vector>
#include <map>
#include <algorithm>

#include <iostream>
#include <glm/glm.hpp>

#include "subdivision.h"

using namespace catmull;

void CatmullSubdivision::
loadMesh(obj_mesh &_obj) {
	edgeList.clear(); faceList.clear(); vertexList.clear();

	std::map<std::pair<int, int>, int> lineMap; // A pair of vertex indices is used as index of lines, and we'll make sure the smaller one is always first
	vertexList.resize(_obj.positions.size());
	// TODO
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
