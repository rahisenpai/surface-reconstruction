#include "contour.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

std::vector<ContourPlane> parseContourFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::vector<ContourPlane> contourPlanes;
    int numPlanes;
    file >> numPlanes;

    contourPlanes.reserve(numPlanes);

    for (int i = 0; i < numPlanes; ++i) {
        ContourPlane contourPlane;
        Plane& plane = contourPlane.plane;
        file >> plane.a >> plane.b >> plane.c >> plane.d;

        file >> contourPlane.numVertices >> contourPlane.numEdges;

        contourPlane.vertices.resize(contourPlane.numVertices);
        for (int j = 0; j < contourPlane.numVertices; ++j) {
            file >> contourPlane.vertices[j].x >> contourPlane.vertices[j].y >> contourPlane.vertices[j].z;
        }

        contourPlane.edges.resize(contourPlane.numEdges);
        for (int j = 0; j < contourPlane.numEdges; ++j) {
            file >> contourPlane.edges[j].vertexIndex1 >> contourPlane.edges[j].vertexIndex2 
                 >> contourPlane.edges[j].materialIndexLeft >> contourPlane.edges[j].materialIndexRight;
        }

        contourPlanes.push_back(contourPlane);
    }

    return contourPlanes;
}

void renderContourPlanes(const std::vector<ContourPlane>& planes) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_LINES);
    for (const auto& plane : planes) {
        for (const auto& edge : plane.edges) {
            const Vertex& v1 = plane.vertices[edge.vertexIndex1];
            const Vertex& v2 = plane.vertices[edge.vertexIndex2];

            // Set color for the edges (e.g., red color)
            glColor3f(1.0f, 0.0f, 0.0f);

            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
        }
    }
    glEnd();
}