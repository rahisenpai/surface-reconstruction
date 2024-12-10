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
    void renderPartitions() const;
    void renderPolyhedron(const CGAL::Polyhedron_3<ExactKernel>& poly) const;
    std::vector<CGAL::Polyhedron_3<ExactKernel>> getConvexCells() { return m_convexCells; }

private:
    void recursivePartition(Nef_polyhedron& space, const std::vector<ContourPlane>& contourPlanes, std::vector<CGAL::Nef_polyhedron_3<ExactKernel>>& nefPolys);
    Nef_polyhedron computeBoundingBox() const;
    std::pair<Point, Point> getBBoxCorners() const;
    std::vector<CGAL::Polyhedron_3<ExactKernel>> m_convexCells;
    std::vector<ContourPlane> m_contourPlanes;
    Nef_polyhedron m_partitionedSpace;
};

#endif