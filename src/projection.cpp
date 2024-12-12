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
    
    for (size_t i = 0; i < m_cells.size(); i++) {
        auto planes = partitioner.getPlanesForCell(i);
        for (const auto& plane : planes) {
            auto it = std::find(m_contourPlanes.begin(), m_contourPlanes.end(), plane);
            if (it == m_contourPlanes.end()) {
                m_contourPlanes.push_back(plane);
            }
        }
        m_cellPlanes[i] = computeAxisAlignedPlanes(m_cells[i].geometry);
    }

    computeProjections();
}

AxisPlanes Projection::computeAxisAlignedPlanes(const CGAL::Polyhedron_3<ExactKernel>& poly) const {
    AxisPlanes result;
    
    auto bbox = CGAL::bounding_box(poly.points_begin(), poly.points_end());
    double xmin = CGAL::to_double(bbox.xmin());
    double ymin = CGAL::to_double(bbox.ymin());
    double zmin = CGAL::to_double(bbox.zmin());
    double xmax = CGAL::to_double(bbox.xmax());
    double ymax = CGAL::to_double(bbox.ymax());
    double zmax = CGAL::to_double(bbox.zmax());

    double xcenter = (xmin + xmax) / 2;
    double ycenter = (ymin + ymax) / 2;
    double zcenter = (zmin + zmax) / 2;

    AxisPlanes::Plane xPlane;
    xPlane.position = xcenter;
    xPlane.axis = 'x';
    xPlane.corners = {
        Point(xcenter, ymin, zmin),
        Point(xcenter, ymax, zmin),
        Point(xcenter, ymax, zmax),
        Point(xcenter, ymin, zmax)
    };
    result.planes.push_back(xPlane);

    AxisPlanes::Plane yPlane;
    yPlane.position = ycenter;
    yPlane.axis = 'y';
    yPlane.corners = {
        Point(xmin, ycenter, zmin),
        Point(xmax, ycenter, zmin),
        Point(xmax, ycenter, zmax),
        Point(xmin, ycenter, zmax)
    };
    result.planes.push_back(yPlane);

    AxisPlanes::Plane zPlane;
    zPlane.position = zcenter;
    zPlane.axis = 'z';
    zPlane.corners = {
        Point(xmin, ymin, zcenter),
        Point(xmax, ymin, zcenter),
        Point(xmax, ymax, zcenter),
        Point(xmin, ymax, zcenter)
    };
    result.planes.push_back(zPlane);

    return result;
}

void Projection::renderAxisPlanes(const AxisPlanes& planes) const {
    glColor3f(1.0f, 0.75f, 0.8f);
    glBegin(GL_QUADS);
    for (const auto& plane : planes.planes) {
        for (const auto& corner : plane.corners) {
            glVertex3d(CGAL::to_double(corner.x()),
                      CGAL::to_double(corner.y()),
                      CGAL::to_double(corner.z()));
        }
    }
    glEnd();
}

void Projection::renderPlanesForAllCells() const {
    for (size_t i = 0; i < m_cells.size(); i++) {
        auto it = m_cellPlanes.find(i);
        if (it != m_cellPlanes.end()) {
            renderAxisPlanes(it->second);
        }
    }
}

void Projection::renderPlanesForCell(const CGAL::Polyhedron_3<ExactKernel>& poly) const {
    size_t cellIndex = 0;
    for (size_t i = 0; i < m_cells.size(); i++) {
        if (&m_cells[i].geometry == &poly) {
            cellIndex = i;
            break;
        }
    }
    
    auto it = m_cellPlanes.find(cellIndex);
    if (it != m_cellPlanes.end()) {
        renderAxisPlanes(it->second);
    }
}

const AxisPlanes& Projection::getAxisPlanesForCell(size_t cellIndex) const {
    auto it = m_cellPlanes.find(cellIndex);
    if (it == m_cellPlanes.end()) {
        static AxisPlanes empty;
        return empty;
    }
    return it->second;
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

double Projection::computePlaneDotProduct(const Plane& contourPlane, 
                                        const AxisPlanes::Plane& axisPlane) const {
    // Get normal vector based on axis
    CGAL::Vector_3<InexactKernel> axisNormal;
    switch(axisPlane.axis) {
        case 'x': axisNormal = CGAL::Vector_3<InexactKernel>(1, 0, 0); break;
        case 'y': axisNormal = CGAL::Vector_3<InexactKernel>(0, 1, 0); break;
        case 'z': axisNormal = CGAL::Vector_3<InexactKernel>(0, 0, 1); break;
        default: return 0.0;
    }
    
    // Get contour plane normal
    CGAL::Vector_3<InexactKernel> contourNormal = contourPlane.orthogonal_vector();
    
    // Normalize vectors
    contourNormal = contourNormal / sqrt(contourNormal.squared_length());
    
    // Return dot product (not absolute value)
    return contourNormal * axisNormal;
}

const AxisPlanes::Plane* Projection::selectProjectionPlane(
    const ContourPlane& contourPlane,
    const AxisPlanes& axisPlanes) const {
    
    double minDist = std::numeric_limits<double>::max();
    const AxisPlanes::Plane* bestPlane = nullptr;
    
    for (const auto& plane : axisPlanes.planes) {
        double dot = computePlaneDotProduct(contourPlane.plane, plane);
        // Find distance from +1 instead of -1
        double distFromOne = std::abs(dot - 1.0);
        if (distFromOne < minDist) {
            minDist = distFromOne;
            bestPlane = &plane;
        }
    }
    
    return bestPlane;
}

std::vector<Point> Projection::projectVerticesOntoPlane(
    const std::vector<Point>& vertices,
    const AxisPlanes::Plane& plane) const {
    
    std::vector<Point> projected;
    projected.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        Point projectedPoint;
        switch(plane.axis) {
            case 'x':
                projectedPoint = Point(plane.position, 
                                     CGAL::to_double(vertex.y()),
                                     CGAL::to_double(vertex.z()));
                break;
            case 'y':
                projectedPoint = Point(CGAL::to_double(vertex.x()),
                                     plane.position,
                                     CGAL::to_double(vertex.z()));
                break;
            case 'z':
                projectedPoint = Point(CGAL::to_double(vertex.x()),
                                     CGAL::to_double(vertex.y()),
                                     plane.position);
                break;
        }
        projected.push_back(projectedPoint);
    }
    
    return projected;
}

ReconstructedMesh Projection::triangulateVertices(const std::vector<Point>& points) const {
    ReconstructedMesh result;
    result.vertices = points;
    
    // Create Delaunay triangulation
    typedef CGAL::Triangulation_3<InexactKernel> Triangulation;
    Triangulation T;
    T.insert(points.begin(), points.end());

    // Create vertex index mapping
    std::map<Point, size_t> vertex_indices;
    for(size_t i = 0; i < points.size(); i++) {
        vertex_indices[points[i]] = i;
    }

    // Extract finite facets
    for(auto fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit) {
        std::array<size_t, 3> triangle;
        
        Triangulation::Cell_handle cell = fit->first;
        int i = fit->second;
        
        for(int j = 0; j < 3; j++) {
            Point p = cell->vertex(T.vertex_triple_index(i, j))->point();
            triangle[j] = vertex_indices[p];
        }
        
        result.triangles.push_back(triangle);
    }

    // Create surface mesh
    for (const auto& p : points) {
        result.mesh.add_vertex(p);
    }
    
    for (const auto& triangle : result.triangles) {
        result.mesh.add_face(
            typename CGAL::Surface_mesh<Point>::Vertex_index(triangle[0]),
            typename CGAL::Surface_mesh<Point>::Vertex_index(triangle[1]), 
            typename CGAL::Surface_mesh<Point>::Vertex_index(triangle[2]));
    }

    return result;
}

void Projection::computeProjections() {
    m_projectedContours.clear();

    for (size_t cellIdx = 0; cellIdx < m_cells.size(); cellIdx++) {
        CellProjections cellProj;
        cellProj.cellIndex = cellIdx;

        auto contourPlanes = getPlanesForCell(cellIdx);
        const auto& axisPlanes = getAxisPlanesForCell(cellIdx);

        // Select a common projection plane for the cell
        const AxisPlanes::Plane* commonProjectionPlane = nullptr;
        if (!contourPlanes.empty()) {
            // For simplicity, select projection plane based on the first contour plane
            commonProjectionPlane = selectProjectionPlane(contourPlanes[0], axisPlanes);
        }

        if (!commonProjectionPlane) {
            continue; // Skip this cell if no projection plane is found
        }

        // Collect all original and projected vertices for this cell
        std::vector<Point> originalVertices;
        std::vector<Point> projectedVertices;

        for (const auto& contourPlane : contourPlanes) {
            originalVertices.insert(originalVertices.end(),
                                    contourPlane.vertices.begin(),
                                    contourPlane.vertices.end());

            std::vector<Point> projVertices = projectVerticesOntoPlane(
                contourPlane.vertices, *commonProjectionPlane);
            projectedVertices.insert(projectedVertices.end(),
                                     projVertices.begin(), projVertices.end());
        }

        if (!originalVertices.empty() && !projectedVertices.empty()) {
            // Reconstruct surface for this cell
            ReconstructedMesh reconstructedMesh = reconstructCellSurface(originalVertices, projectedVertices);

            // Create a single ProjectedContour for the cell
            ProjectedContour proj;
            proj.originalPlane = nullptr; // Since it's the whole cell
            proj.projectionPlane = commonProjectionPlane;
            proj.projectedVertices = projectedVertices;
            proj.reconstructedSurface = reconstructedMesh;

            cellProj.projections.push_back(proj);
            m_projectedContours.push_back(cellProj);
        }
    }
}

ReconstructedMesh Projection::reconstructCellSurface(
    const std::vector<Point>& originalVertices,
    const std::vector<Point>& projectedVertices) const {

    ReconstructedMesh result;

    // Combine original and projected vertices
    std::vector<Point> combinedPoints;
    combinedPoints.reserve(originalVertices.size() + projectedVertices.size());
    combinedPoints.insert(combinedPoints.end(), originalVertices.begin(), originalVertices.end());
    combinedPoints.insert(combinedPoints.end(), projectedVertices.begin(), projectedVertices.end());

    // Perform triangulation on combined points
    typedef CGAL::Triangulation_3<InexactKernel> Triangulation;
    Triangulation T;
    T.insert(combinedPoints.begin(), combinedPoints.end());

    // Create vertex index mapping using std::map instead of unordered_map
    std::map<Point, size_t> vertex_indices;
    for (size_t i = 0; i < combinedPoints.size(); i++) {
        vertex_indices[combinedPoints[i]] = i;
    }

    // Extract triangles from finite facets
    result.triangles.clear();
    for (auto fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit) {
        std::array<size_t, 3> triangle;

        Triangulation::Cell_handle cell = fit->first;
        int i = fit->second;

        for (int j = 0; j < 3; j++) {
            Point p = cell->vertex(T.vertex_triple_index(i, j))->point();
            triangle[j] = vertex_indices[p];
        }

        result.triangles.push_back(triangle);
    }

    // Store vertices
    result.vertices = combinedPoints;

    // Create surface mesh
    CGAL::Surface_mesh<Point>& mesh = result.mesh;
    mesh.clear();

    // Add vertices
    std::vector<typename CGAL::Surface_mesh<Point>::Vertex_index> mesh_vertex_indices;
    for (const auto& p : combinedPoints) {
        mesh_vertex_indices.push_back(mesh.add_vertex(p));
    }

    // Add faces
    for (const auto& triangle : result.triangles) {
        mesh.add_face(mesh_vertex_indices[triangle[0]],
                      mesh_vertex_indices[triangle[1]],
                      mesh_vertex_indices[triangle[2]]);
    }

    return result;
}


void Projection::reconstructSurface(ProjectedContour& projection) {
    // Combine original and projected vertices
    std::vector<Point> combinedPoints;
    combinedPoints.reserve(projection.originalPlane->vertices.size() + 
                         projection.projectedVertices.size());
    
    // Add original vertices
    combinedPoints.insert(combinedPoints.end(),
                         projection.originalPlane->vertices.begin(),
                         projection.originalPlane->vertices.end());
    
    // Add projected vertices
    combinedPoints.insert(combinedPoints.end(),
                         projection.projectedVertices.begin(),
                         projection.projectedVertices.end());

    // Store combined vertices
    projection.reconstructedSurface.vertices = combinedPoints;

    // Perform single triangulation on combined points
    typedef CGAL::Triangulation_3<InexactKernel> Triangulation;
    Triangulation T;
    T.insert(combinedPoints.begin(), combinedPoints.end());

    // Create vertex index mapping
    std::map<Point, size_t> vertex_indices;
    for(size_t i = 0; i < combinedPoints.size(); i++) {
        vertex_indices[combinedPoints[i]] = i;
    }

    // Extract triangles from finite facets
    projection.reconstructedSurface.triangles.clear();
    for(auto fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit) {
        std::array<size_t, 3> triangle;
        
        Triangulation::Cell_handle cell = fit->first;
        int i = fit->second;
        
        for(int j = 0; j < 3; j++) {
            Point p = cell->vertex(T.vertex_triple_index(i, j))->point();
            triangle[j] = vertex_indices[p];
        }
        
        projection.reconstructedSurface.triangles.push_back(triangle);
    }

    // Create surface mesh
    CGAL::Surface_mesh<Point>& mesh = projection.reconstructedSurface.mesh;
    mesh.clear();
    
    // Add vertices
    std::vector<typename CGAL::Surface_mesh<Point>::Vertex_index> mesh_vertex_indices;
    for (const auto& p : combinedPoints) {
        mesh_vertex_indices.push_back(mesh.add_vertex(p));
    }
    
    // Add faces
    for (const auto& triangle : projection.reconstructedSurface.triangles) {
        mesh.add_face(mesh_vertex_indices[triangle[0]], 
                     mesh_vertex_indices[triangle[1]], 
                     mesh_vertex_indices[triangle[2]]);
    }
}

void Projection::renderReconstructedSurface(const ReconstructedMesh& mesh) const {
    // First render triangles
    glColor3f(0.2f, 0.6f, 0.8f);
    
    glBegin(GL_TRIANGLES);
    for(const auto& triangle : mesh.triangles) {
        // Normal computation for depth perception
        const Point& p0 = mesh.vertices[triangle[0]];
        const Point& p1 = mesh.vertices[triangle[1]];
        const Point& p2 = mesh.vertices[triangle[2]];
        
        CGAL::Vector_3<InexactKernel> v1(p1.x() - p0.x(), p1.y() - p0.y(), p1.z() - p0.z());
        CGAL::Vector_3<InexactKernel> v2(p2.x() - p0.x(), p2.y() - p0.y(), p2.z() - p0.z());
        auto normal = CGAL::cross_product(v1, v2);
        normal = normal / std::sqrt(normal.squared_length());

        glNormal3d(CGAL::to_double(normal.x()),
                  CGAL::to_double(normal.y()),
                  CGAL::to_double(normal.z()));

        for(size_t idx : triangle) {
            const Point& p = mesh.vertices[idx];
            glVertex3d(CGAL::to_double(p.x()),
                      CGAL::to_double(p.y()),
                      CGAL::to_double(p.z()));
        }
    }
    glEnd();

    // Then render edges
    glLineWidth(1.5f);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black edges
    
    glBegin(GL_LINES);
    for(const auto& triangle : mesh.triangles) {
        // Draw three edges of each triangle
        for(int i = 0; i < 3; i++) {
            const Point& p1 = mesh.vertices[triangle[i]];
            const Point& p2 = mesh.vertices[triangle[(i + 1) % 3]];
            
            glVertex3d(CGAL::to_double(p1.x()),
                      CGAL::to_double(p1.y()),
                      CGAL::to_double(p1.z()));
            glVertex3d(CGAL::to_double(p2.x()),
                      CGAL::to_double(p2.y()),
                      CGAL::to_double(p2.z()));
        }
    }
    glEnd();
    
    glLineWidth(1.0f);  // Reset line width
}

void Projection::renderAllReconstructions() const {
    for (const auto& cellProj : m_projectedContours) {
        for (const auto& proj : cellProj.projections) {
            renderReconstructedSurface(proj.reconstructedSurface);
        }
    }
}