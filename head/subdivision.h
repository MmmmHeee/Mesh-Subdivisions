#pragma once

#include "obj_mesh.h"

class Subdivision {
    protected:
        virtual void subdiv() = 0; // do subdivision for once
        virtual obj_mesh makeMesh() = 0; // give the obj mesh from data used for subdivisions
    public:
        virtual void loadMesh(obj_mesh & _obj) = 0; // prepare data structure for subdivisions
        obj_mesh execute(unsigned int _times = 3) {
            for (unsigned int i = 0; i < _times; i += 1) {
                subdiv();
            }
            return makeMesh();
        }
};

#include "subdivisions/loop.h"
class LoopSubdivision : public Subdivision {
    private:
        std::vector<loop::vertex> vertexList;
        std::vector<loop::edge> edgeList;
        std::vector<loop::face> faceList;
    protected:
        void subdiv() override;
        obj_mesh makeMesh() override;
    public:
        void loadMesh(obj_mesh & _obj) override;
};

#include "subdivisions/catmull.h"
class CatmullSubdivision : public Subdivision {
	private:
		std::vector<catmull::vertex> vertexList;
		std::vector<catmull::edge> edgeList;
		std::vector<catmull::face> faceList;
	protected:
		void subdiv() override;
		obj_mesh makeMesh() override;
	public:
		void loadMesh(obj_mesh & _obj) override;
	
		//Note: In this Catmull Clark implementation, we ignored uv & normal to save some time. In fact they can easily be calculated just like pos.
};

#include "subdivisions/doosabin.h"
class DoosabinSubdivision : public Subdivision {
private:
	std::vector<doosabin::vertex> vertexList;
	std::vector<doosabin::edge> edgeList;
	std::vector<doosabin::face> faceList;
protected:
	void subdiv() override;
	obj_mesh makeMesh() override;
	obj_mesh doosabin_makeMesh();
public:
	void loadMesh(obj_mesh & _obj) override;
	obj_mesh execute_doosabin(unsigned int _times = 3) {
		obj_mesh tmp;
		for (unsigned int i = 0; i < _times; i += 1) {
			subdiv();
			obj_mesh tmp_mesh = makeMesh();
			writeObj("test.obj", tmp_mesh);
			loadObj("test.obj", tmp);
			loadMesh(tmp);
			writeObj("output.obj", tmp_mesh);
		}
		return tmp;
	}
	

	//No more normal calculate;
};