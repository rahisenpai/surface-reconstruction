#include "contour.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

std::vector<ContourPlane> parseContourFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::vector<ContourPlane> contourPlanes;
    int numPlanes;
    file >> numPlanes;

    for (int i = 0; i < numPlanes; ++i) {
        float a, b, c, d;
        file >> a >> b >> c >> d; // Read plane coefficients
        Plane plane(a, b, c, d);

        ContourPlane contourPlane;
        contourPlane.plane = plane;

        int numVertices, numEdges;
        file >> numVertices >> numEdges;

        for (int j = 0; j < numVertices; ++j) {
            float x, y, z;
            file >> x >> y >> z;
            contourPlane.vertices.emplace_back(x, y, z);
        }

        for (int j = 0; j < numEdges; ++j) {
            int v1, v2, m1, m2;
            file >> v1 >> v2 >> m1 >> m2;
            contourPlane.edges.emplace_back(v1, v2);
        }

        contourPlanes.push_back(contourPlane);
    }

    return contourPlanes;
}

void renderContourPlanes(const std::vector<ContourPlane>& contourPlanes) {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set drawing color to Red
    glColor3f(1.0f, 0.0f, 0.0f);
    
    for (const auto& contourPlane : contourPlanes) {
        glBegin(GL_LINES);
        for (const auto& edge : contourPlane.edges) {
            const Point& p1 = contourPlane.vertices[edge.first];
            const Point& p2 = contourPlane.vertices[edge.second];
            glVertex3f(p1.x(), p1.y(), p1.z());
            glVertex3f(p2.x(), p2.y(), p2.z());
        }
        glEnd();
    }
}