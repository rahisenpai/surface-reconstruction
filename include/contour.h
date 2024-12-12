// contour.h
#ifndef CONTOUR_H
#define CONTOUR_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Extended_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Plane_3.h>
#include <GL/glew.h>

typedef CGAL::Extended_cartesian<CGAL::Gmpq> ExactKernel;
typedef CGAL::Exact_predicates_inexact_constructions_kernel InexactKernel;

typedef InexactKernel::Point_3 Point;
typedef InexactKernel::Plane_3 Plane;
typedef CGAL::Polyhedron_3<InexactKernel> Polyhedron;

typedef ExactKernel::Point_3 ExactPoint;
typedef ExactKernel::Plane_3 ExactPlane;
typedef CGAL::Polyhedron_3<ExactKernel> ExactPolyhedron;

struct ExtendedMesh {
    std::vector<Point> vertices;
    struct Face {
        size_t v1, v2, v3;
        int materialPos, materialNeg;
    };
    std::vector<Face> faces;
    std::vector<std::pair<size_t, size_t>> contourEdges;
};

struct ContourPlane {
    Plane plane;
    std::vector<Point> vertices;
    std::vector<std::pair<int, int>> edges;
    std::string filename;
    bool hasExt = false;
    ExtendedMesh extMesh;

    bool operator==(const ContourPlane& other) const {
        return plane == other.plane &&
               vertices == other.vertices &&
               edges == other.edges &&
               filename == other.filename;
    }
};

std::vector<ContourPlane> parseContourFile(const std::string& filePath);
void renderContourPlanes(const std::vector<ContourPlane>& planes);
void renderExtendedMesh(const ExtendedMesh& mesh);

#endif