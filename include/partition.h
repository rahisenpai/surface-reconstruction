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

private:
    Nef_polyhedron computeBoundingBox() const;
    std::pair<Point, Point> getBBoxCorners() const;
    
    std::vector<ContourPlane> m_contourPlanes;
    Nef_polyhedron m_partitionedSpace;
};

#endif