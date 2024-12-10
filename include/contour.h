// contour.h
#ifndef CONTOUR_H
#define CONTOUR_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Plane_3.h>
#include <GL/glew.h>

// Kernel definitions
typedef CGAL::Extended_cartesian<CGAL::Gmpq> ExactKernel;
typedef CGAL::Exact_predicates_inexact_constructions_kernel InexactKernel;

// Type definitions for input/rendering (inexact)
typedef InexactKernel::Point_3 Point;
typedef InexactKernel::Plane_3 Plane;
typedef CGAL::Polyhedron_3<InexactKernel> Polyhedron;

// Type definitions for exact computations
typedef ExactKernel::Point_3 ExactPoint;
typedef ExactKernel::Plane_3 ExactPlane;
typedef CGAL::Polyhedron_3<ExactKernel> ExactPolyhedron;

struct ContourPlane {
    Plane plane;
    std::vector<Point> vertices;
    std::vector<std::pair<int, int>> edges;
};

std::vector<ContourPlane> parseContourFile(const std::string& filePath);
void renderContourPlanes(const std::vector<ContourPlane>& planes);

#endif