#include "contour.h"
#include <iostream>
#include <fstream>

std::vector<ContourPlane> parseContourFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    int numPlanes;
    file >> numPlanes;

    std::vector<ContourPlane> planes;
    planes.reserve(numPlanes);

    for (int i = 0; i < numPlanes; ++i) {
        ContourPlane plane;
        file >> plane.a >> plane.b >> plane.c >> plane.d;

        int numVertices, numEdges;
        file >> numVertices >> numEdges;

        plane.vertices.resize(numVertices);
        for (int j = 0; j < numVertices; ++j) {
            file >> plane.vertices[j].x >> plane.vertices[j].y >> plane.vertices[j].z;
        }

        plane.edges.resize(numEdges);
        for (int j = 0; j < numEdges; ++j) {
            file >> plane.edges[j].vi1 >> plane.edges[j].vi2 >> plane.edges[j].mi1 >> plane.edges[j].mi2;
        }

        planes.push_back(plane);
    }

    return planes;
}

void renderContourPlanes(const std::vector<ContourPlane>& planes) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_LINES);
    for (const auto& plane : planes) {
        for (const auto& edge : plane.edges) {
            const Vertex& v1 = plane.vertices[edge.vi1];
            const Vertex& v2 = plane.vertices[edge.vi2];

            // Set color for the edges (e.g., red color)
            glColor3f(1.0f, 0.0f, 0.0f);

            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
        }
    }
    glEnd();
}