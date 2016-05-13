#ifndef MESH_H
#define MESH_H

#include "Types.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "HalfEdge.h"
#include <Eigen/SparseCore>
#include <Eigen/SparseCholesky>

class Mesh {
public:
    // default constructor
    Mesh();
        
    // read mesh from file
    bool read(const std::string& fileName);
    
    // write mesh to file
    bool write(const std::string& fileName) const;
    
    // computes wave flow
    void computeWaveFlow(const double min, const double max);
    
    // setups up flow
    void setup(const double h0, const double a0, const double b0);
        
    // member variables
    std::vector<HalfEdge> halfEdges;
    std::vector<Vertex> vertices;
    std::vector<Eigen::Vector3d> uvs;
    std::vector<Eigen::Vector3d> normals;
    std::vector<Edge> edges;
    std::vector<Face> faces;
    std::vector<HalfEdgeIter> boundaries;

private:
    // ((1 + bh)A - adt^2 L)
    void buildWaveOperator(Eigen::SparseMatrix<double>& A, const double a) const;
    
    // center mesh about origin and rescale to unit radius
    void normalize();
    
    double h;
    double b;
    Eigen::SimplicialCholesky<Eigen::SparseMatrix<double>> solver;
};

#endif