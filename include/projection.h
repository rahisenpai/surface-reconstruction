// projection.h
#ifndef PROJECTION_H
#define PROJECTION_H

#include "contour.h"
#include "partition.h"

class Projection {
public:
    // Constructor takes partitioner to access cells
    Projection(const SpacePartitioner& partitioner);

    // Access methods
    size_t getCellCount() const { return m_cells.size(); }
    const std::vector<SpacePartitioner::ConvexCell>& getCells() const { return m_cells; }
    std::vector<ContourPlane> getPlanesForCell(size_t cellIndex) const;
    void debugPrintCellInfo() const;
    void renderPlanesForAllCells();

private:
    std::vector<SpacePartitioner::ConvexCell> m_cells;
    std::vector<ContourPlane> m_contourPlanes;
    void renderPlanesForCell(const CGAL::Polyhedron_3<ExactKernel>& poly) const;
};

#endif