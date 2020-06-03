#include "obj_mesh.h"
#include "subdivision.h"
int main()
{
	obj_mesh obj;
	loadObj("models/tetrahedron.obj", obj);

	LoopSubdivision loop;
	loop.loadMesh(obj);
	obj = loop.execute(3);
	writeObj("build/output.obj", obj);

	CatmullSubdivision catmull;
	catmull.loadMesh(obj);

	DoosabinSubdivision doosabin;
	doosabin.loadMesh(obj);
	return 0;
}