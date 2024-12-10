// partition.cpp
#include "partition.h"
#include <CGAL/bounding_box.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Cartesian_converter.h>
#include <fstream>
#include <iostream>
#include <CGAL/IO/Polyhedron_OFF_iostream.h>
#include <filesystem>
namespace fs = std::filesystem;

std::string SpacePartitioner::getConvexCellsPath(const std::string& contourName) const {
    return "../data/convex_cells/" + contourName;
}

void SpacePartitioner::ensureDirectoryExists(const std::string& path) const {
    fs::create_directories(path);
}

bool SpacePartitioner::loadConvexCells(const std::string& contourName) {
    std::string cellsDir = getConvexCellsPath(contourName);
    if (!fs::exists(cellsDir)) {
        return false;
    }

    m_convexCells.clear();
    size_t cellCount = 0;
    bool success = true;

    for (const auto& entry : fs::directory_iterator(cellsDir)) {
        if (entry.path().extension() == ".off") {
            CGAL::Polyhedron_3<ExactKernel> poly;
            std::ifstream file(entry.path());
            if (file && CGAL::read_off(file, poly)) {
                m_convexCells.push_back(poly);
                cellCount++;
            } else {
                success = false;
                break;
            }
        }
    }

    if (!success || cellCount == 0) {
        m_convexCells.clear();
        return false;
    }

    std::cout << "Loaded " << cellCount << " convex cells from " << cellsDir << std::endl;
    return true;
}

void SpacePartitioner::saveConvexCells(const std::string& contourName) const {
    if (m_convexCells.empty()) return;

    std::string cellsDir = getConvexCellsPath(contourName);
    ensureDirectoryExists(cellsDir);

    for (size_t i = 0; i < m_convexCells.size(); ++i) {
        std::string filename = cellsDir + "/cell_" + std::to_string(i) + ".off";
        std::ofstream file(filename);
        if (file) {
            CGAL::write_off(file, m_convexCells[i]);
        }
    }

    std::cout << "Saved " << m_convexCells.size() << " convex cells to " << cellsDir << std::endl;
}

// Converter between kernels
typedef CGAL::Cartesian_converter<InexactKernel, ExactKernel> IK_to_EK;
typedef CGAL::Cartesian_converter<ExactKernel, InexactKernel> EK_to_IK;

SpacePartitioner::SpacePartitioner(const std::vector<ContourPlane>& contourPlanes)
    : m_contourPlanes(contourPlanes) {}

std::pair<Point, Point> SpacePartitioner::getBBoxCorners() const {
    std::vector<Point> allPoints;
    for (const auto& contourPlane : m_contourPlanes) {
        allPoints.insert(allPoints.end(), 
                        contourPlane.vertices.begin(),
                        contourPlane.vertices.end());
    }
    
    auto bbox = CGAL::bounding_box(allPoints.begin(), allPoints.end());
    
    // Add padding (10% of bbox diagonal)
    double dx = bbox.xmax() - bbox.xmin();
    double dy = bbox.ymax() - bbox.ymin();
    double dz = bbox.zmax() - bbox.zmin();
    double padding = 0.1 * std::sqrt(dx*dx + dy*dy + dz*dz);
    
    return std::make_pair(
        Point(bbox.xmin() - padding, bbox.ymin() - padding, bbox.zmin() - padding),
        Point(bbox.xmax() + padding, bbox.ymax() + padding, bbox.zmax() + padding)
    );
}

Nef_polyhedron SpacePartitioner::computeBoundingBox() const {
    auto [min_corner, max_corner] = getBBoxCorners();
    
    // Convert to exact kernel
    IK_to_EK to_exact;
    
    std::vector<ExactKernel::Point_3> exact_vertices;
    exact_vertices.reserve(8);
    
    // Convert vertices to exact kernel
    std::vector<Point> vertices = {
        Point(min_corner.x(), min_corner.y(), min_corner.z()),
        Point(max_corner.x(), min_corner.y(), min_corner.z()),
        Point(min_corner.x(), max_corner.y(), min_corner.z()),
        Point(max_corner.x(), max_corner.y(), min_corner.z()),
        Point(min_corner.x(), min_corner.y(), max_corner.z()),
        Point(max_corner.x(), min_corner.y(), max_corner.z()),
        Point(min_corner.x(), max_corner.y(), max_corner.z()),
        Point(max_corner.x(), max_corner.y(), max_corner.z())
    };
    
    for (const auto& v : vertices) {
        exact_vertices.push_back(to_exact(v));
    }

    // Create exact polyhedron
    CGAL::Polyhedron_3<ExactKernel> exact_poly;
    CGAL::convex_hull_3(exact_vertices.begin(), exact_vertices.end(), exact_poly);
    return Nef_polyhedron(exact_poly);
}

void SpacePartitioner::partition() {
    // Extract contour name from first plane for directory naming
    std::string contourName = fs::path(fs::path(m_contourPlanes[0].filename).stem()).string();
    
    // Try to load existing cells first
    if (loadConvexCells(contourName)) {
        return;
    }

    // Compute new partition if loading failed
    std::cout << "Computing new partition for " << contourName << "..." << std::endl;

    // Precompute exact planes once
    precomputePlanes();

    m_partitionedSpace = computeBoundingBox();
    std::vector<Nef_polyhedron> nefPolys;
    

    // Filter elementary cells
    std::vector<Nef_polyhedron> elementaryPolys;

    partitionSpace(m_partitionedSpace, 0, nefPolys);
    
    for (size_t i = 0; i < nefPolys.size(); ++i) {
        bool isElementary = true;
        
        // Convert to Polyhedron to compute volume
        CGAL::Polyhedron_3<ExactKernel> poly_i;
        nefPolys[i].convert_to_polyhedron(poly_i);
        
        for (size_t j = 0; j < nefPolys.size(); ++j) {
            if (i != j) {
                CGAL::Polyhedron_3<ExactKernel> poly_j;
                nefPolys[j].convert_to_polyhedron(poly_j);
                
                // Check intersection
                Nef_polyhedron intersection = nefPolys[i] * nefPolys[j];
                
                if (!intersection.is_empty()) {
                    CGAL::Polyhedron_3<ExactKernel> poly_intersection;
                    intersection.convert_to_polyhedron(poly_intersection);
                    
                    // Compare number of vertices and facets instead of volume
                    if (poly_intersection.size_of_vertices() == poly_i.size_of_vertices() ||
                        poly_intersection.size_of_vertices() == poly_j.size_of_vertices()) {
                        if (poly_i.size_of_vertices() <= poly_j.size_of_vertices()) {
                            isElementary = false;
                            break;
                        }
                    }
                }
            }
        }
        
        if (isElementary) {
            elementaryPolys.push_back(nefPolys[i]);
        }
    }

    // Convert to regular polyhedra
    for (const auto& cell : elementaryPolys) {
        CGAL::Polyhedron_3<ExactKernel> poly;
        cell.convert_to_polyhedron(poly);
        m_convexCells.push_back(poly);
    }

    saveConvexCells(contourName);
}

void SpacePartitioner::precomputePlanes() {
    IK_to_EK to_exact;
    m_exactPlanes.reserve(m_contourPlanes.size());
    for (const auto& plane : m_contourPlanes) {
        m_exactPlanes.push_back(to_exact(plane.plane));
    }
}

void SpacePartitioner::partitionSpace(
    Nef_polyhedron& space,
    size_t planeIndex,
    std::vector<Nef_polyhedron>& nefPolys) {
    
    // Early termination conditions
    if (space.is_empty() || space.number_of_vertices() == 0) {
        return;
    }
    
    if (planeIndex >= m_exactPlanes.size()) {
        nefPolys.push_back(space);
        return;
    }
    
    // Get precomputed exact plane
    const ExactKernel::Plane_3& exact_plane = m_exactPlanes[planeIndex];
    
    // Create halfspaces
    Nef_polyhedron plane_nef(exact_plane, Nef_polyhedron::INCLUDED);
    
    // Partition space
    Nef_polyhedron positive_space = space * plane_nef;
    
    // Only compute complement if needed
    if (!positive_space.is_empty() && positive_space.number_of_vertices() > 0) {
        partitionSpace(positive_space, planeIndex + 1, nefPolys);
    }
    
    // Reuse original space for negative half-space
    space *= plane_nef.complement();
    if (!space.is_empty() && space.number_of_vertices() > 0) {
        partitionSpace(space, planeIndex + 1, nefPolys);
    }
}


void SpacePartitioner::renderPolyhedron(const CGAL::Polyhedron_3<ExactKernel>& poly) const {
    EK_to_IK to_inexact;
    glColor3f(0.0f, 0.0f, 1.0f); 
    glLineWidth(2.0f);

    for (auto e = poly.edges_begin(); e != poly.edges_end(); ++e) {
        Point v1 = to_inexact(e->vertex()->point());
        Point v2 = to_inexact(e->opposite()->vertex()->point());

        glBegin(GL_LINES);
        glVertex3d(CGAL::to_double(v1.x()), CGAL::to_double(v1.y()), CGAL::to_double(v1.z()));
        glVertex3d(CGAL::to_double(v2.x()), CGAL::to_double(v2.y()), CGAL::to_double(v2.z()));
        glEnd();
    }
}
