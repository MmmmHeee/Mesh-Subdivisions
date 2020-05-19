#include "obj_mesh.h"
#include "subdivision.h"
int main()
{
	obj_mesh obj;
	loadObj("models/tetrahedron.obj", obj);
	LoopSubdivision loop;
	loop.loadMesh(obj);
	obj = loop.execute(4);
	writeObj("build/output.obj", obj);

	return 0;
}