#include "obj_mesh.h"
#include "subdivision.h"
int main()
{
	obj_mesh obj;
	loadObj("../models/tetrahedron.obj", obj);
	obj_mesh obj_2;
	loadObj("../models/quad.obj", obj_2);
	//LoopSubdivision loop;
	//loop.loadMesh(obj);
	//obj = loop.execute(3);
	//writeObj("../build/output.obj", obj);


	CatmullSubdivision catmull;
	catmull.loadMesh(obj);
	obj = catmull.execute(3);
	writeObj("../build/catmull_opt.obj", obj);
	return 0;
}