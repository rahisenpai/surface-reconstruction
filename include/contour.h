#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>
#include <string>
#include <GL/glew.h>

struct Vertex {
    float x, y, z;
};

struct Edge {
    int vi1, vi2, mi1, mi2;
};

struct ContourPlane {
    float a, b, c, d;
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
};

std::vector<ContourPlane> parseContourFile(const std::string& filePath);
void renderContourPlanes(const std::vector<ContourPlane>& planes);

#endif // CONTOUR_H