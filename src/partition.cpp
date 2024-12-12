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
    if (!fs::exists(cellsDir)) return false;

    m_cells.clear();
    size_t cellCount = 0;

    for (const auto& entry : fs::directory_iterator(cellsDir)) {
        if (entry.path().extension() == ".off") {
            ConvexCell cell;
            
            // Load geometry
            std::ifstream geomFile(entry.path());
            if (!geomFile || !CGAL::read_off(geomFile, cell.geometry)) {
                return false;
            }

            // Load plane associations
            std::string planesPath = entry.path().string();
            planesPath.replace(planesPath.end()-4, planesPath.end(), ".planes");
            std::ifstream planesFile(planesPath);
            size_t planeIdx;
            while (planesFile >> planeIdx) {
                cell.planeIndices.push_back(planeIdx);
            }

            m_cells.push_back(cell);
            cellCount++;
        }
    }

    return cellCount > 0;
}

void SpacePartitioner::saveConvexCells(const std::string& contourName) const {
    if (m_cells.empty()) return;

    std::string cellsDir = getConvexCellsPath(contourName);
    ensureDirectoryExists(cellsDir);

    for (size_t i = 0; i < m_cells.size(); ++i) {
        // Save geometry
        std::string offFile = cellsDir + "/cell_" + std::to_string(i) + ".off";
        std::ofstream geomFile(offFile);
        if (geomFile) {
            CGAL::write_off(geomFile, m_cells[i].geometry);
        }

        // Save plane associations
        std::string planesFile = cellsDir + "/cell_" + std::to_string(i) + ".planes";
        std::ofstream planeFile(planesFile);
        if (planeFile) {
            for (size_t idx : m_cells[i].planeIndices) {
                planeFile << idx << " ";
            }
        }
    }
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
    std::string contourName = fs::path(m_contourPlanes[0].filename).stem().string();
    
    if (loadConvexCells(contourName)) {
        return;
    }

    std::cout << "Computing partition for " << contourName << "..." << std::endl;
    precomputePlanes();
    m_partitionedSpace = computeBoundingBox();
    
    std::vector<std::pair<Nef_polyhedron, std::set<size_t>>> nefPolys;
    partitionSpace(m_partitionedSpace, 0, nefPolys);

    // Filter elementary cells
    m_cells.clear();
    for (const auto& [nef, planeSet] : nefPolys) {
        bool isElementary = true;
        CGAL::Polyhedron_3<ExactKernel> poly_i;
        nef.convert_to_polyhedron(poly_i);
        
        for (const auto& [other_nef, other_set] : nefPolys) {
            if (&nef != &other_nef) {
                Nef_polyhedron intersection = nef * other_nef;
                if (!intersection.is_empty()) {
                    CGAL::Polyhedron_3<ExactKernel> poly_intersection;
                    intersection.convert_to_polyhedron(poly_intersection);
                    
                    if (poly_intersection.size_of_vertices() == poly_i.size_of_vertices()) {
                        isElementary = false;
                        break;
                    }
                }
            }
        }
        
        if (isElementary) {
            ConvexCell cell;
            nef.convert_to_polyhedron(cell.geometry);
            cell.planeIndices.insert(cell.planeIndices.end(), 
                                   planeSet.begin(), planeSet.end());
            m_cells.push_back(cell);
        }
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
    std::vector<std::pair<Nef_polyhedron, std::set<size_t>>>& nefPolys) {
    
    if (space.is_empty() || space.number_of_vertices() == 0) {
        return;
    }
    
    if (planeIndex >= m_exactPlanes.size()) {
        nefPolys.push_back({space, std::set<size_t>()});
        return;
    }
    
    const ExactKernel::Plane_3& exact_plane = m_exactPlanes[planeIndex];
    Nef_polyhedron plane_nef(exact_plane, Nef_polyhedron::INCLUDED);
    
    Nef_polyhedron positive_space = space * plane_nef;
    if (!positive_space.is_empty() && positive_space.number_of_vertices() > 0) {
        std::set<size_t> pos_planes;
        if (!nefPolys.empty()) {
            pos_planes = nefPolys.back().second;
        }
        pos_planes.insert(planeIndex);
        partitionSpace(positive_space, planeIndex + 1, nefPolys);
        if (!nefPolys.empty()) {
            nefPolys.back().second = pos_planes;
        }
    }
    
    space *= plane_nef.complement();
    if (!space.is_empty() && space.number_of_vertices() > 0) {
        partitionSpace(space, planeIndex + 1, nefPolys);
    }
}

std::vector<ContourPlane> SpacePartitioner::getPlanesForCell(size_t cellIndex) const {
    if (cellIndex >= m_cells.size()) return {};

    std::vector<ContourPlane> planes;
    for (size_t idx : m_cells[cellIndex].planeIndices) {
        if (idx < m_contourPlanes.size()) {
            planes.push_back(m_contourPlanes[idx]);
        }
    }
    return planes;
}


void SpacePartitioner::renderPolyhedron(const ConvexCell& cell, bool highlight) const {
    EK_to_IK to_inexact;
    
    if (highlight) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red for highlighted cells
    } else {
        glColor3f(0.0f, 0.0f, 1.0f); // Blue for normal cells
    }
    glLineWidth(2.0f);

    for (auto e = cell.geometry.edges_begin(); e != cell.geometry.edges_end(); ++e) {
        Point v1 = to_inexact(e->vertex()->point());
        Point v2 = to_inexact(e->opposite()->vertex()->point());

        glBegin(GL_LINES);
        glVertex3d(CGAL::to_double(v1.x()), 
                  CGAL::to_double(v1.y()), 
                  CGAL::to_double(v1.z()));
        glVertex3d(CGAL::to_double(v2.x()), 
                  CGAL::to_double(v2.y()), 
                  CGAL::to_double(v2.z()));
        glEnd();
    }
}
