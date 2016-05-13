#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"

std::vector<HalfEdge> isolated;

bool Vertex::isIsolated() const
{
    return he == isolated.begin();
}

double Vertex::dualArea() const
{    
    double area = 0.0;
    
    HalfEdgeCIter h = he;
    do {
        area += h->face->area();
        h = h->flip->next;
        
    } while (h != he);
    
    return area / 3.0;
}

void Vertex::computeNormal() 
{
    Eigen::Vector3d n;
    normal.setZero();
    
    double angle;
    
    HalfEdgeCIter h = he;
    do {
        Eigen::Vector3d e1 = h->flip->vertex->position - position;
        Eigen::Vector3d e2 = h->flip->next->flip->vertex->position - position;
        
        double c = e1.dot(e2) / sqrt(e1.dot(e1) * e2.dot(e2));
        if (c < -1.0) c = -1.0;
        else if (c >  1.0) c = 1.0;
        angle = acos(c);
        
        normal += angle * h->face->normal();
        h = h->flip->next;
        
    } while (h != he);
    
    normal.normalize();
}
