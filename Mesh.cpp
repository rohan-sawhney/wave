#include "Mesh.h"
#include "MeshIO.h"

Mesh::Mesh()
{
    
}

bool Mesh::read(const std::string& fileName)
{
    std::ifstream in(fileName.c_str());

    if (!in.is_open()) {
        std::cerr << "Error: Could not open file for reading" << std::endl;
        return false;
    }
    
    bool readSuccessful = false;
    if ((readSuccessful = MeshIO::read(in, *this))) {
        normalize();
    }
    
    return readSuccessful;
}

bool Mesh::write(const std::string& fileName) const
{
    std::ofstream out(fileName.c_str());
    
    if (!out.is_open()) {
        std::cerr << "Error: Could not open file for writing" << std::endl;
        return false;
    }
    
    MeshIO::write(out, *this);
    
    return false;
}

void Mesh::buildWaveOperator(Eigen::SparseMatrix<double>& A, const double a) const
{
    double fact = 1 + b*h;
    double ah2 = a*h*h;
    std::vector<Eigen::Triplet<double>> ATriplet;
    
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        
        HalfEdgeCIter he = v->he;
        double dualArea = v->dualArea();
        double sumCoefficients = 0.0;

        do {
            double coefficient = -(he->cotan() + he->flip->cotan())*ah2*0.5;
            
            ATriplet.push_back(Eigen::Triplet<double>(v->index, he->flip->vertex->index, coefficient));
            sumCoefficients += coefficient;
            
            he = he->flip->next;
        } while (he != v->he);
        
        ATriplet.push_back(Eigen::Triplet<double>(v->index, v->index, fact*dualArea - sumCoefficients));
    }
    
    A.setFromTriplets(ATriplet.begin(), ATriplet.end());
}

void Mesh::setup(const double h0, const double a0, const double b0)
{
    h = h0;
    b = b0;
    int v = (int)vertices.size();
    
    // build flow operator
    Eigen::SparseMatrix<double> A(v, v);
    buildWaveOperator(A, a0);
    
    solver.compute(A);
}

double shift(const double s, const double min, const double max)
{
    return 0.005 * (s - min) / (max - min);
}

void Mesh::computeWaveFlow(const double min, const double max)
{
    // ((1 + bh)A - ah^2 L) c(t) = (A((2 - bh) * c(t-h) - c(t-2h)));
    int v = (int)vertices.size();
    double fact = 2 - b*h;
    
    // set right hand side
    Eigen::VectorXd colors(v);
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        colors(v->index) = v->dualArea() * (fact*v->currColor - v->prevColor);
    }
    
    // solve
    colors = solver.solve(colors);
    
    // update vertex positions
    for (VertexIter v = vertices.begin(); v != vertices.end(); v++) {
        v->prevColor = v->currColor;
        v->currColor = colors(v->index);
        v->shiftedPosition = v->position + shift(v->currColor, min, max) * v->normal;
    }
}

void Mesh::normalize()
{
    // compute center of mass
    Eigen::Vector3d cm = Eigen::Vector3d::Zero();
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        cm += v->position;
    }
    cm /= (double)vertices.size();
    
    // translate to origin and determine radius
    double rMax = 0;
    for (VertexIter v = vertices.begin(); v != vertices.end(); v++) {
        v->position -= cm;
        rMax = std::max(rMax, v->position.norm());
    }
    
    // rescale to unit sphere
    for (VertexIter v = vertices.begin(); v != vertices.end(); v++) {
        v->position /= rMax;
    }
}
