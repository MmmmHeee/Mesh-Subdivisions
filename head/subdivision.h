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
		std::vector<loop::vertex> vertexList;
		std::vector<loop::edge> edgeList;
		std::vector<loop::face> faceList;
	protected:
		void subdiv() override;
		obj_mesh makeMesh() override;
	public:
		void loadMesh(obj_mesh & _obj) override;
};