// partition.h
#ifndef PARTITION_H
#define PARTITION_H

#include <CGAL/Nef_polyhedron_3.h>
#include "contour.h"

typedef CGAL::Nef_polyhedron_3<ExactKernel> Nef_polyhedron;

class SpacePartitioner {
public:
    SpacePartitioner(const std::vector<ContourPlane>& contourPlanes);
    void partition();
    bool loadConvexCells(const std::string& contourName);
    void saveConvexCells(const std::string& contourName) const;
    void renderPolyhedron(const CGAL::Polyhedron_3<ExactKernel>& poly) const;
    std::vector<CGAL::Polyhedron_3<ExactKernel>> getConvexCells() { return m_convexCells; }

private:
    std::string getConvexCellsPath(const std::string& contourName) const;
    void ensureDirectoryExists(const std::string& path) const;
    std::vector<ExactKernel::Plane_3> m_exactPlanes;
    void precomputePlanes();
    void partitionSpace(Nef_polyhedron& space, 
                       size_t planeIndex,
                       std::vector<Nef_polyhedron>& nefPolys);
    Nef_polyhedron computeBoundingBox() const;
    std::pair<Point, Point> getBBoxCorners() const;
    std::vector<CGAL::Polyhedron_3<ExactKernel>> m_convexCells;
    std::vector<ContourPlane> m_contourPlanes;
    Nef_polyhedron m_partitionedSpace;
};

#endif