// partition.cpp
#include "partition.h"
#include <CGAL/bounding_box.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Cartesian_converter.h>

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
    m_partitionedSpace = computeBoundingBox();
    
    IK_to_EK to_exact;
    
    for (const auto& contourPlane : m_contourPlanes) {
        // Convert inexact plane to exact plane
        ExactKernel::Plane_3 exact_plane = to_exact(contourPlane.plane);
        Nef_polyhedron plane_nef(exact_plane);
        m_partitionedSpace *= plane_nef;
    }
}

void SpacePartitioner::renderPartitions() const {
    if (m_partitionedSpace.is_empty()) {
        return;
    }

    EK_to_IK to_inexact;
    CGAL::Polyhedron_3<ExactKernel> exact_poly;
    m_partitionedSpace.convert_to_polyhedron(exact_poly);

    glColor3f(0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    
    for (auto e = exact_poly.edges_begin(); e != exact_poly.edges_end(); ++e) {
        // Convert vertices back to inexact for rendering
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