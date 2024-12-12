// projection.cpp
#include "projection.h"
#include <iostream>

Projection::Projection(const SpacePartitioner& partitioner) {
    m_cells = partitioner.getConvexCells();
    
    // Store unique planes
    for (size_t i = 0; i < m_cells.size(); i++) {
        auto planes = partitioner.getPlanesForCell(i);
        for (const auto& plane : planes) {
            // Use std::find_if with a lambda for custom comparison if needed
            auto it = std::find(m_contourPlanes.begin(), m_contourPlanes.end(), plane);
            if (it == m_contourPlanes.end()) {
                m_contourPlanes.push_back(plane);
            }
        }
    }
}

std::vector<ContourPlane> Projection::getPlanesForCell(size_t cellIndex) const {
    if (cellIndex >= m_cells.size()) return {};

    std::vector<ContourPlane> planes;
    for (size_t planeIdx : m_cells[cellIndex].planeIndices) {
        if (planeIdx < m_contourPlanes.size()) {
            planes.push_back(m_contourPlanes[planeIdx]);
        }
    }
    return planes;
}

void Projection::debugPrintCellInfo() const {
    std::cout << "Projection contains " << m_cells.size() << " cells:" << std::endl;
    for (size_t i = 0; i < m_cells.size(); i++) {
        auto planes = getPlanesForCell(i);
        std::cout << "Cell " << i << " uses " << planes.size() 
                  << " planes" << std::endl;
    }
}