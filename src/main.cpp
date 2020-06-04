#include "obj_mesh.h"
#include "subdivision.h"
int main()
{
	obj_mesh obj;
	loadObj("cube_quad.obj", obj);
	obj_mesh obj_2;
	loadObj("../models/quad.obj", obj_2);
	//LoopSubdivision loop;
	//loop.loadMesh(obj);
	//obj = loop.execute(3);
	//writeObj("../build/output.obj", obj);


	DoosabinSubdivision doosabin;
	doosabin.loadMesh(obj);
	obj = doosabin.execute_doosabin(2);
	//writeObj("output.obj", obj);
	return 0;
}