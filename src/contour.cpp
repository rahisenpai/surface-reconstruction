// contour.cpp
#include "contour.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

std::vector<ContourPlane> parseContourFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    std::vector<ContourPlane> contourPlanes;
    int numPlanes;
    file >> numPlanes;

    for (int i = 0; i < numPlanes; ++i)
    {
        float a, b, c, d;
        file >> a >> b >> c >> d;
        Plane plane(a, b, c, d);

        ContourPlane contourPlane;
        contourPlane.filename = filePath;
        contourPlane.plane = plane;
        contourPlane.hasExt = false;

        int numVertices, numEdges;
        file >> numVertices >> numEdges;

        for (int j = 0; j < numVertices; ++j)
        {
            float x, y, z;
            file >> x >> y >> z;
            contourPlane.vertices.emplace_back(x, y, z);
        }

        for (int j = 0; j < numEdges; ++j)
        {
            int v1, v2, m1, m2;
            file >> v1 >> v2 >> m1 >> m2;
            contourPlane.edges.emplace_back(v1, v2);
        }

        // Read potential whitespace and next character
        char marker;
        file >> std::ws; // Skip whitespace
        if (file.get(marker))
        {
            if (marker == '~')
            {
                std::cout << "Found extended mesh data" << std::endl;
                int numVerts, numFaces;
                file >> numVerts >> numFaces;

                // Read vertices
                for (int j = 0; j < numVerts; ++j)
                {
                    float x, y, z;
                    file >> x >> y >> z;
                    contourPlane.extMesh.vertices.emplace_back(x, y, z);
                }

                // Read faces
                for (int j = 0; j < numFaces; ++j)
                {
                    ExtendedMesh::Face face;
                    file >> face.v1 >> face.v2 >> face.v3 >> face.materialPos >> face.materialNeg;
                    contourPlane.extMesh.faces.push_back(face);
                }

                // Read number of contour edges
                int numContourEdges;
                file >> numContourEdges;

                // Read contour edges
                for (int j = 0; j < numContourEdges; ++j)
                {
                    size_t v1, v2;
                    file >> v1 >> v2;
                    contourPlane.extMesh.contourEdges.emplace_back(v1, v2);
                }

                contourPlane.hasExt = true;
            }
            else
            {
                // Put back the character if it wasn't a '~'
                file.unget();
            }
        }

        contourPlanes.push_back(contourPlane);
    }

    return contourPlanes;
}

void renderContourPlanes(const std::vector<ContourPlane> &contourPlanes)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto &contourPlane : contourPlanes)
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        for (const auto &edge : contourPlane.edges)
        {
            const Point &p1 = contourPlane.vertices[edge.first];
            const Point &p2 = contourPlane.vertices[edge.second];
            glVertex3f(p1.x(), p1.y(), p1.z());
            glVertex3f(p2.x(), p2.y(), p2.z());
        }
        glEnd();
    }
}