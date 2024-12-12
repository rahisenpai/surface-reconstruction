// projection.cpp
#include "projection.h"
#include <iostream>
#include <vector>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/bounding_box.h>
#include <CGAL/Cartesian_converter.h>
#include <GL/glew.h>
#include "partition.h"

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

void Projection::renderPlanesForAllCells() {
    for (const auto& cell : m_cells) {
        renderPlanesForCell(cell.geometry);
    }
}

void Projection::renderPlanesForCell(const CGAL::Polyhedron_3<ExactKernel>& poly) const {
    // Compute bounding box for the polyhedron
    auto bbox = CGAL::bounding_box(poly.points_begin(), poly.points_end());

    // Extract bounding box min and max points
    double xmin = CGAL::to_double(bbox.xmin());
    double ymin = CGAL::to_double(bbox.ymin());
    double zmin = CGAL::to_double(bbox.zmin());
    double xmax = CGAL::to_double(bbox.xmax());
    double ymax = CGAL::to_double(bbox.ymax());
    double zmax = CGAL::to_double(bbox.zmax());

    // Define corners of the planes for rendering
    struct Corner {
        double x, y, z;
    };

    std::vector<std::vector<Corner>> planes = {
        // // Bottom plane (z = zmin)
        // {{xmin, ymin, zmin}, {xmax, ymin, zmin}, {xmax, ymax, zmin}, {xmin, ymax, zmin}},
        // // Top plane (z = zmax)
        // {{xmin, ymin, zmax}, {xmax, ymin, zmax}, {xmax, ymax, zmax}, {xmin, ymax, zmax}},
        // // Left plane (x = xmin)
        // {{xmin, ymin, zmin}, {xmin, ymax, zmin}, {xmin, ymax, zmax}, {xmin, ymin, zmax}},
        // // Right plane (x = xmax)
        // {{xmax, ymin, zmin}, {xmax, ymax, zmin}, {xmax, ymax, zmax}, {xmax, ymin, zmax}},
        // // Front plane (y = ymin)
        // {{xmin, ymin, zmin}, {xmax, ymin, zmin}, {xmax, ymin, zmax}, {xmin, ymin, zmax}},
        // // Back plane (y = ymax)
        // {{xmin, ymax, zmin}, {xmax, ymax, zmin}, {xmax, ymax, zmax}, {xmin, ymax, zmax}},

        // Bottom plane (z = zmin) + Top plane (z = zmax)
        {{xmin, ymin, (zmin+zmax)/2}, {xmax, ymin, (zmin+zmax)/2}, {xmax, ymax, (zmin+zmax)/2}, {xmin, ymax, (zmin+zmax)/2}},
        // Left plane (x = xmin) + Right plane (x = xmax)
        {{(xmin+xmax)/2, ymin, zmin}, {(xmin+xmax)/2, ymax, zmin}, {(xmin+xmax)/2, ymax, zmax}, {(xmin+xmax)/2, ymin, zmax}},
        // Front plane (y = ymin) + Back plane (y = ymax)
        {{xmin, (ymin+ymax)/2, zmin}, {xmax, (ymin+ymax)/2, zmin}, {xmax, (ymin+ymax)/2, zmax}, {xmin, (ymin+ymax)/2, zmax}},
    };

    // Render planes
    glColor3f(1.0f, 0.75f, 0.8f);
    glBegin(GL_QUADS);
    for (const auto& plane : planes) {
        for (const auto& corner : plane) {
            glVertex3d(corner.x, corner.y, corner.z);
        }
    }
    glEnd();
}