// partition.h
#ifndef PARTITION_H
#define PARTITION_H

#include <CGAL/Nef_polyhedron_3.h>
#include "contour.h"
#include <set>

typedef CGAL::Nef_polyhedron_3<ExactKernel> Nef_polyhedron;

class SpacePartitioner {
public:
    struct ConvexCell {
        CGAL::Polyhedron_3<ExactKernel> geometry;
        std::vector<size_t> planeIndices;  // Indices of defining planes
    };

    SpacePartitioner(const std::vector<ContourPlane>& contourPlanes);
    void partition();
    bool loadConvexCells(const std::string& contourName);
    void saveConvexCells(const std::string& contourName) const;
    void renderPolyhedron(const ConvexCell& cell, bool highlight = false) const;
    const std::vector<ConvexCell>& getConvexCells() const { return m_cells; }
    std::vector<ContourPlane> getPlanesForCell(size_t cellIndex) const;

private:
    std::string getConvexCellsPath(const std::string& contourName) const;
    void ensureDirectoryExists(const std::string& path) const;
    std::vector<ExactKernel::Plane_3> m_exactPlanes;
    void precomputePlanes();
    void partitionSpace(Nef_polyhedron& space, 
                       size_t planeIndex,
                       std::vector<std::pair<Nef_polyhedron, std::set<size_t>>>& nefPolys);
    Nef_polyhedron computeBoundingBox() const;
    std::pair<Point, Point> getBBoxCorners() const;
    
    std::vector<ConvexCell> m_cells;
    std::vector<ContourPlane> m_contourPlanes;
    Nef_polyhedron m_partitionedSpace;
};

#endif