#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>
#include <string>
#include <GL/glew.h>

struct Plane {
    float a, b, c, d;

    // Default constructor
    Plane() : a(0), b(0), c(0), d(0) {}

    // Constructor with parameters
    Plane(float a_, float b_, float c_, float d_)
        : a(a_), b(b_), c(c_), d(d_) {}
};

struct Vertex {
    float x, y, z;
    std::vector<int> associatedPlanes; // Indices of planes intersecting at this vertex
    std::vector<int> associatedEdges;  // Indices of edges connected to this vertex

    Vertex() : x(0), y(0), z(0) {}
    Vertex(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_) {}
};

struct Edge {
    int vertexIndex1, vertexIndex2;   // Indices of the two endpoints
    int materialIndexLeft, materialIndexRight; // Indices of the materials on the left and right sides
    int planeIndex1, planeIndex2;     // Indices of the two planes defining the edge
};

struct Face {
    int planeIndex;                   // Index of the defining plane
    std::vector<int> edgeIndices;     // Indices of edges forming the boundary
};

struct ContourPlane {
    Plane plane;
    int numVertices;
    int numEdges;
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
};

std::vector<ContourPlane> parseContourFile(const std::string& filePath);
void renderContourPlanes(const std::vector<ContourPlane>& planes);

#endif // CONTOUR_H