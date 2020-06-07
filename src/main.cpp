#include "obj_mesh.h"
#include "subdivision.h"
int main()
{
	obj_mesh tetrahedron;
	loadObj("models/tetrahedron.obj", tetrahedron);
	
	obj_mesh cube;
	loadObj("models/cube_quad.obj", cube);
	
	obj_mesh cube_tri;
	loadObj("models/cube_tri.obj", cube_tri);

	std::string OUT_PATH = "output/";

	LoopSubdivision loop;
	loop.loadMesh(tetrahedron);
	writeObj(OUT_PATH + "loop_tetrahedron_1.obj", loop.execute(1));
	writeObj(OUT_PATH + "loop_tetrahedron_2.obj", loop.execute(1));
	writeObj(OUT_PATH + "loop_tetrahedron_3.obj", loop.execute(1));

	loop.loadMesh(cube_tri);
	writeObj(OUT_PATH + "loop_cube_tri_1.obj", loop.execute(1));
	writeObj(OUT_PATH + "loop_cube_tri_2.obj", loop.execute(1));
	writeObj(OUT_PATH + "loop_cube_tri_3.obj", loop.execute(1));

	CatmullSubdivision catmull;
	catmull.loadMesh(tetrahedron);
	writeObj(OUT_PATH + "catmull_tetrahedron_1.obj", catmull.execute(1));
	writeObj(OUT_PATH + "catmull_tetrahedron_2.obj", catmull.execute(1));
	writeObj(OUT_PATH + "catmull_tetrahedron_3.obj", catmull.execute(1));

	catmull.loadMesh(cube);
	writeObj(OUT_PATH + "catmull_cube_quad_1.obj", catmull.execute(1));
	writeObj(OUT_PATH + "catmull_cube_quad_2.obj", catmull.execute(1));
	writeObj(OUT_PATH + "catmull_cube_quad_3.obj", catmull.execute(1));

	Doosabin2Subdivision doosabin;
	doosabin.loadMesh(tetrahedron);
	writeObj(OUT_PATH + "doosabin_tetrahedron_1.obj", doosabin.execute(1));
	writeObj(OUT_PATH + "doosabin_tetrahedron_2.obj", doosabin.execute(1));
	writeObj(OUT_PATH + "doosabin_tetrahedron_3.obj", doosabin.execute(1));

	doosabin.loadMesh(cube);
	writeObj(OUT_PATH + "doosabin_cube_quad_1.obj", doosabin.execute(1));
	writeObj(OUT_PATH + "doosabin_cube_quad_2.obj", doosabin.execute(1));
	writeObj(OUT_PATH + "doosabin_cube_quad_3.obj", doosabin.execute(1));

	return 0;
}